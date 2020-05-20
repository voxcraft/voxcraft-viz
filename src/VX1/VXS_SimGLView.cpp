/*******************************************************************************
Copyright (c) 2010, Jonathan Hiller (Cornell University)
If used in publication cite "J. Hiller and H. Lipson "Dynamic Simulation of Soft
Heterogeneous Objects" In press. (2011)"

This file is part of Voxelyze.
Voxelyze is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version. Voxelyze is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
details. See <http://www.opensource.org/licenses/lgpl-3.0.html> for license
details.
*******************************************************************************/
#include "VXS_SimGLView.h"
#include "GL_Utils.h"
#include <QDebug>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
namespace pt = boost::property_tree;

#ifdef VX2
// Voxelyze/include
#include "VX_Link.h"
#include "VX_MeshRender.h"
#include "VX_Voxel.h"
#endif

CVXS_SimGLView::CVXS_SimGLView(CVX_Sim *pSimIn) {
    pSim = pSimIn;

    CurViewMode = RVM_VOXELS;
    CurViewCol = RVC_TYPE;
    CurViewVox = RVV_DEFORMED;
    //	CurViewVox = RVV_DISCRETE;

    ViewForce = false;
    ViewAngles = false;

    NeedStatsUpdate = true;

    defMeshVx2 = new CVX_MeshRender(&pSimIn->Vx);

    HistoryPaused = false;
}

CVXS_SimGLView::~CVXS_SimGLView(void) {}

CVXS_SimGLView &CVXS_SimGLView::operator=(const CVXS_SimGLView &rGlView) // overload "="
{
    pSim = rGlView.pSim;
    NeedStatsUpdate = rGlView.NeedStatsUpdate;
    ViewForce = rGlView.ViewForce;
    ViewAngles = rGlView.ViewAngles;
    CurViewMode = rGlView.CurViewMode;
    CurViewCol = rGlView.CurViewCol;
    CurViewVox = rGlView.CurViewVox;

    return *this;
}

void CVXS_SimGLView::Draw(int Selected, bool ViewSection, int SectionLayer) {
    if (!pSim->IsInitalized())
        return;

    if (CurViewMode == RVM_NONE)
        return;
    else if (CurViewMode == RVM_VOXELS) {
        switch (CurViewVox) {
        case RVV_DISCRETE:
            DrawGeometry(Selected, ViewSection, SectionLayer);
            break; // section view only currently enabled in voxel view mode
        case RVV_DEFORMED:
            DrawVoxMesh(Selected);
            break;
        case RVV_SMOOTH:
            DrawSurfMesh();
            break;
        case RVV_HISTORY:
            DrawHistory(Selected);
            break;
        case RVV_HISTORY_ELECTRICAL:
            DrawHistory(Selected, RVV_HISTORY_ELECTRICAL);
            break;
        case RVV_HISTORY_ROTATION:
            DrawHistory(Selected, RVV_HISTORY_ROTATION);
            break;
        }
    } else { // CurViewMode == RVT_BONDS
        vfloat VoxScale = 0.2;
        if (ViewForce) {
            DrawForce();
            DrawGeometry(Selected, ViewSection, SectionLayer, VoxScale);

        } else {
            if (CurViewVox == RVV_SMOOTH)
                VoxScale = 0.1;
            DrawBonds();
            DrawGeometry(Selected, ViewSection, SectionLayer, VoxScale);
        }
        DrawStaticFric();
    }
    if (ViewAngles)
        DrawAngles();
    if (pSim->IsFeatureEnabled(VXSFEAT_FLOOR))
        DrawFloor(); // draw the floor if its in use
    //	if (pEnv->IsFloorEnabled()) DrawFloor(); //draw the floor if its in use

    NeedStatsUpdate = true;
}

void CVXS_SimGLView::DrawForce(void) {
    //	#ifndef VX2
    CVoxelyze *pVx = &pSim->Vx;
    CVX_Voxel *pV;

    float PrevLineWidth;
    glGetFloatv(GL_LINE_WIDTH, &PrevLineWidth);
    glLineWidth(1.0);
    glDisable(GL_LIGHTING);

    vfloat MaxForce = 0;
    int iT = pVx->linkCount();
    int NumVox = pVx->voxelCount();
    if (NumVox <= 0)
        return;
    float vSize = (float)pVx->voxelSize();

    for (int i = 0; i < iT; i++) { // go through all the bonds...
        vfloat ThisMag = pVx->link(i)->force(true).Length();
        if (ThisMag > MaxForce)
            MaxForce = ThisMag;
    }

    vfloat ForceScale = 0.4 * vSize / MaxForce; // Vox size / max Force

    //	int x, y, z;
    glBegin(GL_LINES);
    glLoadName(-1);                  // to disable picking
    for (int i = 0; i < NumVox; i++) // go through all the voxels...
    {
        // pSim->pEnv->pObj->GetXYZNom(&x, &y, &z, pSim->StoXIndexMap[i]);
        // if (ViewSection && z>SectionLayer) continue; //exit if obscured in a
        // section view!

        pV = pVx->voxel(i); // &pSim->VoxArray[i];

        Vec3D<> Center = pV->position();    // ox->GetCurPos();
        Quat3D<> Angle = pV->orientation(); // pVox->GetCurAngle();
        Vec3D<> PointToDrawFrom;
        for (int i = 0; i < 6; i++) // for each potential bond
        {
            switch (i) {
            case BD_PX:
                PointToDrawFrom = Center + Angle.RotateVec3D(Vec3D<>(0.1 * vSize, 0, 0));
                break;
            case BD_NX:
                PointToDrawFrom = Center + Angle.RotateVec3D(Vec3D<>(-0.1 * vSize, 0, 0));
                break;
            case BD_PY:
                PointToDrawFrom = Center + Angle.RotateVec3D(Vec3D<>(0, 0.1 * vSize, 0));
                break;
            case BD_NY:
                PointToDrawFrom = Center + Angle.RotateVec3D(Vec3D<>(0, -0.1 * vSize, 0));
                break;
            case BD_PZ:
                PointToDrawFrom = Center + Angle.RotateVec3D(Vec3D<>(0, 0, 0.1 * vSize));
                break;
            case BD_NZ:
                PointToDrawFrom = Center + Angle.RotateVec3D(Vec3D<>(0, 0, -0.1 * vSize));
                break;
            };

            //			CVXS_BondInternal* pBond =
            // pVox->GetpInternalBond((BondDir)i);
            CVX_Link *pLink = pV->link((CVX_Voxel::linkDirection)i);
            if (pLink) {
                glColor4d(1.0, 0.0, 0.0, 1.0); // red
                Vec3D<> PointToDrawTo;

                // Total Force
                PointToDrawTo = PointToDrawFrom + ForceScale * pLink->force(pV == pLink->voxel(true));
                //				if (pVox->IAmInternalVox2(i))
                // PointToDrawTo =
                // PointToDrawFrom+ForceScale*pBond->GetForce2();
                // else
                // PointToDrawTo =
                // PointToDrawFrom+ForceScale*pBond->GetForce1();
                CGL_Utils::DrawLineArrowD(PointToDrawFrom, PointToDrawTo, 2.0, CColor(1, 0, 0)); // Red

                ////Axial Force
                // if (pVox->IAmInternalVox2(i)) PointToDrawTo =
                // PointToDrawFrom+ForceScale*pBond->AxialForce2; else
                // PointToDrawTo =
                // PointToDrawFrom+ForceScale*pBond->AxialForce1;
                // CGL_Utils::DrawLineArrowD(PointToDrawFrom,
                // PointToDrawTo, 1.0, CColor(0.2, 0.2, 0.7)); //Blue

                ////Bending Force
                // if (pVox->IAmInternalVox2(i)) PointToDrawTo =
                // PointToDrawFrom+ForceScale*pBond->BendingForce2; else
                // PointToDrawTo =
                // PointToDrawFrom+ForceScale*pBond->BendingForce1;
                // CGL_Utils::DrawLineArrowD(PointToDrawFrom,
                // PointToDrawTo, 1.0, CColor(0.2, 0.7, 0.2)); //Green

                ////Shear Force
                // if (pVox->IAmInternalVox2(i)) PointToDrawTo =
                // PointToDrawFrom+ForceScale*pBond->ShearForce2; else
                // PointToDrawTo =
                // PointToDrawFrom+ForceScale*pBond->ShearForce1;
                // CGL_Utils::DrawLineArrowD(PointToDrawFrom,
                // PointToDrawTo, 1.0, CColor(0.7, 0.2, 0.7)); //Purple
            }
        }
    }
    glEnd();

    glLineWidth(PrevLineWidth);
    glEnable(GL_LIGHTING);
    //#endif
}

void CVXS_SimGLView::DrawFloor(void) {
    double z = 0.0;

#ifdef VX2
    if (CurViewVox == RVV_HISTORY ||
        CurViewVox == RVV_HISTORY_ELECTRICAL ||
        CurViewVox == RVV_HISTORY_ROTATION) { // TODO: if showing history, should use history's voxel size. Since we didn't pass this in
                                                // history file, hard coded for now.
        z = -voxel_size / 2;
    } else {
        z = -pSim->Vx.voxelSize() / 2;
    }
#endif

    // TODO: build an openGL list
    vfloat Size = pSim->LocalVXC.GetLatticeDim() * 4;
    vfloat sX = 1.5 * Size;
    vfloat sY = .866 * Size;

    glEnable(GL_LIGHTING);

    glLoadName(-1); // never want to pick floor

    glNormal3d(0.0, 0.0, 1.0);
    for (int i = -20; i <= 30; i++) {
        for (int j = -40; j <= 60; j++) {
            // Draw Hexagons with different colors
            double colorFloor = 0.8 + 0.1 * ((int)(1000 * sin((float)(i + 110) * (j + 106) * (j + 302))) % 10) / 10.0;
            glColor4d(colorFloor, colorFloor, colorFloor + 0.1, 1.0);
            glBegin(GL_TRIANGLE_FAN);
            glVertex3d(i * sX, j * sY, z);
            glVertex3d(i * sX + 0.5 * Size, j * sY, z);
            glVertex3d(i * sX + 0.25 * Size, j * sY + 0.433 * Size, z);
            glVertex3d(i * sX - 0.25 * Size, j * sY + 0.433 * Size, z);
            glVertex3d(i * sX - 0.5 * Size, j * sY, z);
            glVertex3d(i * sX - 0.25 * Size, j * sY - 0.433 * Size, z);
            glVertex3d(i * sX + 0.25 * Size, j * sY - 0.433 * Size, z);
            glVertex3d(i * sX + 0.5 * Size, j * sY, z);
            glEnd();

            colorFloor = 0.8 + 0.1 * ((int)(1000 * sin((float)(i + 100) * (j + 103) * (j + 369))) % 10) / 10.0;
            glColor4d(colorFloor, colorFloor, colorFloor + 0.1, 1.0);

            glBegin(GL_TRIANGLE_FAN);
            glVertex3d(i * sX + .75 * Size, j * sY + 0.433 * Size, z);
            glVertex3d(i * sX + 1.25 * Size, j * sY + 0.433 * Size, z);
            glVertex3d(i * sX + Size, j * sY + 0.866 * Size, z);
            glVertex3d(i * sX + 0.5 * Size, j * sY + 0.866 * Size, z);
            glVertex3d(i * sX + 0.25 * Size, j * sY + 0.433 * Size, z);
            glVertex3d(i * sX + 0.5 * Size, j * sY, z);
            glVertex3d(i * sX + Size, j * sY, z);
            glVertex3d(i * sX + 1.25 * Size, j * sY + 0.433 * Size, z);
            glEnd();
        }
    }
}

void CVXS_SimGLView::DrawGeometry(int Selected, bool ViewSection, int SectionLayer, vfloat ScaleVox) {
#ifdef VX2
    CVoxelyze *pVx = &pSim->Vx;
    const std::vector<CVX_Voxel *> *voxels = pVx->voxelList();
    int indexCounter = 0;
    //	for (std::vector<CVX_Voxel*>::const_iterator it = voxels->begin();
    // it!=voxels->end(); it++){
    for (int i = 0; i < (int)voxels->size(); i++) {

        CVX_Voxel *pVox = pVx->voxel(i); // voxels[i];
                                         //		CVX_Voxel* pVox = *it;
        // glColor4d(ThisColor.r, ThisColor.g, ThisColor.b, ThisColor.a);

        glPushMatrix();
        glTranslated(pVox->position().x, pVox->position().y, pVox->position().z);
        glRotated(pVox->orientation().AngleDegrees(), pVox->orientation().x, pVox->orientation().y, pVox->orientation().z);
        CColor ThisColor = GetCurVoxColor(pVox, indexCounter == Selected);
        glLoadName(indexCounter++); // to enable picking
        CGL_Utils::DrawCube((Vec3D<>)pVox->cornerOffset(CVX_Voxel::NNN) * ScaleVox, (Vec3D<>)pVox->cornerOffset(CVX_Voxel::PPP) * ScaleVox,
                            true, true, 1.0, ThisColor);
        //		CGL_Utils::DrawCube(pVox->cornerNegative()*ScaleVox,
        // pVox->cornerPositive()*ScaleVox, true, true, 1.0,
        // CColor(pVox->material()->red()/255.0,
        // pVox->material()->green()/255.0,
        // pVox->material()->blue()/255.0, 1.0));
        glPopMatrix();
    }
#else
    Vec3D<> Center;
    Vec3D<> tmp(0, 0, 0);

    int iT = pSim->NumVox();
    int x, y, z;
    CColor ThisColor;
    for (int i = 0; i < iT; i++) // go through all the voxels...
    {
        pSim->pEnv->pObj->GetXYZNom(&x, &y, &z, pSim->StoXIndexMap[i]);
        if (ViewSection && z > SectionLayer)
            continue; // exit if obscured in a section view!

        Center = pSim->VoxArray[i].GetCurPos();

        ThisColor = GetCurVoxColor(i, Selected);
        glColor4d(ThisColor.r, ThisColor.g, ThisColor.b, ThisColor.a);

        Vec3D<> CenterOff = ScaleVox * (pSim->VoxArray[i].GetCornerPos() + pSim->VoxArray[i].GetCornerNeg()) / 2;

        glPushMatrix();
        glTranslated(Center.x + CenterOff.x, Center.y + CenterOff.y, Center.z + CenterOff.z);

        glLoadName(pSim->StoXIndexMap[i]); // to enable picking

        // generate rotation matrix here!!! (from quaternion)
        Vec3D<> Axis;
        vfloat AngleAmt;
        Quat3D<>(pSim->VoxArray[i].GetCurAngle()).AngleAxis(AngleAmt, Axis);
        glRotated(AngleAmt * 180 / 3.1415926, Axis.x, Axis.y, Axis.z);

        Vec3D<> CurrentSizeDisplay = pSim->VoxArray[i].GetSizeCurrent();
        glScaled(CurrentSizeDisplay.x, CurrentSizeDisplay.y, CurrentSizeDisplay.z);

        pSim->LocalVXC.Voxel.DrawVoxel(&Center, ScaleVox); // draw unit size since we scaled just now

        glPopMatrix();
    }
#endif
}

#ifdef VX2

CColor CVXS_SimGLView::GetCurVoxColor(CVX_Voxel *pVox, bool Selected) {
    if (Selected)
        return CColor(1.0f, 0.0f, 1.0f,
                      1.0f); // highlight selected voxel (takes precedence...)

    switch (CurViewCol) {
    case RVC_TYPE: {
        float R, G, B, A;
        CVX_Material *pMat = pVox->material();
        return CColor(pMat->red() / 255.0, pMat->green() / 255.0, pMat->blue() / 255.0, pMat->alpha() / 255.0);
    }
    case RVC_KINETIC_EN:
        if (pSim->SS.MaxVoxKinE == 0)
            return GetJet(0);
        return GetJet(pVox->kineticEnergy() / pSim->SS.MaxVoxKinE); // kineticEnergy() / pSim->SS.MaxVoxKinE);
    case RVC_DISP:
        if (pSim->SS.MaxVoxDisp == 0)
            return GetJet(0);
        return GetJet(pVox->displacementMagnitude() / pSim->SS.MaxVoxDisp);
    case RVC_STATE:
        if (pVox->isFailed())
            return CColor(1.0f, 0.0f, 0.0f, 1.0f);
        else if (pVox->isYielded())
            return CColor(1.0f, 1.0f, 0.0f, 1.0f);
        else
            return CColor(1.0f, 1.0f, 1.0f, 1.0f);
    case RVC_STRAIN_EN: {
        if (pSim->SS.MaxBondStrainE == 0)
            return GetJet(0);
        float maxSE = -FLT_MAX;
        for (int i = 0; i < 6; i++) {
            CVX_Link *pL = pVox->link((CVX_Voxel::linkDirection)i);
            if (pL && pL->strainEnergy() > maxSE)
                maxSE = pL->strainEnergy();
        }
        return GetJet(maxSE / pSim->SS.MaxBondStrainE);
    }
    case RVC_STRAIN: {
        if (pSim->SS.MaxBondStrain == 0)
            return GetJet(0);
        float maxSE = -FLT_MAX;
        for (int i = 0; i < 6; i++) {
            CVX_Link *pL = pVox->link((CVX_Voxel::linkDirection)i);
            if (pL && pL->axialStrain() > maxSE)
                maxSE = pL->axialStrain();
        }
        return GetJet(maxSE / pSim->SS.MaxBondStrain);
    }
    case RVC_STRESS: {
        if (pSim->SS.MaxBondStress == 0)
            return GetJet(0);
        float maxSE = -FLT_MAX;
        for (int i = 0; i < 6; i++) {
            CVX_Link *pL = pVox->link((CVX_Voxel::linkDirection)i);
            if (pL && pL->axialStress() > maxSE)
                maxSE = pL->axialStress();
        }
        return GetJet(maxSE / pSim->SS.MaxBondStress);
    }
    case RVC_PRESSURE: {
        float MaxP = pSim->SS.MaxPressure, MinP = pSim->SS.MinPressure;
        if (MaxP <= MinP)
            return GetJet(0);

        float Mag = MaxP;
        if (-MinP > Mag)
            Mag = -MinP;
        // vfloat ThisP = pSim->VoxArray[SIndex].GetPressure();
        return GetJet(0.5 - pVox->pressure() / (2 * Mag));
    }
    default:
        return CColor(1.0f, 1.0f, 1.0f, 1.0f);
    }
}
#endif

CColor CVXS_SimGLView::GetCurVoxColor(int SIndex, int Selected) {
#ifndef VX2
    if (pSim->StoXIndexMap[SIndex] == Selected)
        return CColor(1.0f, 0.0f, 1.0f,
                      1.0f); // highlight selected voxel (takes precedence...)

    switch (CurViewCol) {
    case RVC_TYPE:
        float R, G, B, A;
        //			LocalVXC.GetLeafMat(VoxArray[SIndex].GetVxcIndex())->GetColorf(&R,
        //&G, &B, &A);
        pSim->VoxArray[SIndex].GetpMaterial()->GetColorf(&R, &G, &B, &A);
        return CColor(R, G, B, A);
        break;
    case RVC_KINETIC_EN:
        if (pSim->SS.MaxVoxKinE == 0)
            return GetJet(0);
        return GetJet(pSim->VoxArray[SIndex].GetCurKineticE() / pSim->SS.MaxVoxKinE);
        break;
    case RVC_DISP:
        if (pSim->SS.MaxVoxDisp == 0)
            return GetJet(0);
        return GetJet(pSim->VoxArray[SIndex].GetCurAbsDisp() / pSim->SS.MaxVoxDisp);
        break;
    case RVC_STATE:
        if (pSim->VoxArray[SIndex].GetBroken())
            return CColor(1.0f, 0.0f, 0.0f, 1.0f);
        else if (pSim->VoxArray[SIndex].GetYielded())
            return CColor(1.0f, 1.0f, 0.0f, 1.0f);
        else
            return CColor(1.0f, 1.0f, 1.0f, 1.0f);
        break;
    case RVC_STRAIN_EN:
        if (pSim->SS.MaxBondStrainE == 0)
            return GetJet(0);
        return GetJet(pSim->VoxArray[SIndex].GetMaxBondStrainE() / pSim->SS.MaxBondStrainE);
        break;
    case RVC_STRAIN:
        if (pSim->SS.MaxBondStrain == 0)
            return GetJet(0);
        return GetJet(pSim->VoxArray[SIndex].GetMaxBondStrain() / pSim->SS.MaxBondStrain);
        break;
    case RVC_STRESS:
        if (pSim->SS.MaxBondStress == 0)
            return GetJet(0);
        return GetJet(pSim->VoxArray[SIndex].GetMaxBondStress() / pSim->SS.MaxBondStress);
        break;
    case RVC_PRESSURE: {
        vfloat MaxP = pSim->SS.MaxPressure, MinP = pSim->SS.MinPressure;
        if (MaxP <= MinP)
            return GetJet(0);

        vfloat Mag = MaxP;
        if (-MinP > Mag)
            Mag = -MinP;
        // vfloat ThisP = pSim->VoxArray[SIndex].GetPressure();
        return GetJet(0.5 - pSim->VoxArray[SIndex].GetPressure() / (2 * Mag));
        break;
    }
    default:
        return CColor(1.0f, 1.0f, 1.0f, 1.0f);
        break;
    }
#endif
    return CColor(1.0f, 1.0f, 1.0f, 1.0f);
}

CColor CVXS_SimGLView::GetInternalBondColor(CVX_Link *pLink) {
    switch (CurViewCol) {
    case RVC_TYPE:
        if (pLink->isSmallAngle())
            return CColor(0.3, 0.7, 0.3, 1.0);
        else
            return CColor(0.0, 0.0, 0.0, 1.0);
        break;
    case RVC_KINETIC_EN: {
        if (pSim->SS.MaxVoxKinE == 0)
            return GetJet(0);
        float KePos = pLink->voxel(true)->kineticEnergy();
        float KeNeg = pLink->voxel(false)->kineticEnergy();

        return GetJet((KePos > KeNeg ? KePos : KeNeg) / pSim->SS.MaxVoxKinE);
        break;
    }
    case RVC_DISP: {
        if (pSim->SS.MaxVoxDisp == 0)
            return GetJet(0);
        float dPos = pLink->voxel(true)->displacementMagnitude();
        float dNeg = pLink->voxel(false)->displacementMagnitude();
        return GetJet((dPos > dNeg ? dPos : dNeg) / pSim->SS.MaxVoxDisp);
        break;
    }
    case RVC_STATE:
        if (pLink->isFailed())
            return CColor(1.0f, 0.0f, 0.0f, 1.0f);
        else if (pLink->isYielded())
            return CColor(1.0f, 1.0f, 0.0f, 1.0f);
        else
            return CColor(0.8f, 0.8f, 0.8f, 1.0f);
        break;
    case RVC_STRAIN_EN:
        if (pSim->SS.MaxBondStrainE == 0)
            return GetJet(0);
        return GetJet(pLink->strainEnergy() / pSim->SS.MaxBondStrainE);
        break;
    case RVC_STRAIN:
        if (pSim->SS.MaxBondStrain == 0)
            return GetJet(0);
        return GetJet(pLink->axialStrain() / pSim->SS.MaxBondStrain);
        break;
    case RVC_STRESS:
        if (pSim->SS.MaxBondStress == 0)
            return GetJet(0);
        return GetJet(pLink->axialStress() / pSim->SS.MaxBondStress);
        break;
    case RVC_PRESSURE:
        return GetJet(0); // for now. Pressure of a bond doesn't really make sense
        break;
    default:
        return CColor(0.0f, 0.0f, 0.0f, 1.0f);
        break;
    }
}

#ifndef VX2
CColor CVXS_SimGLView::GetInternalBondColor(CVXS_BondInternal *pBond) {
    switch (CurViewCol) {
    case RVC_TYPE:
        if (pBond->IsSmallAngle())
            return CColor(0.3, 0.7, 0.3, 1.0);
        else
            return CColor(0.0, 0.0, 0.0, 1.0);
        break;
    case RVC_KINETIC_EN:
        if (pSim->SS.MaxVoxKinE == 0)
            return GetJet(0);
        return GetJet(pBond->GetMaxVoxKinE() / pSim->SS.MaxVoxKinE);
        break;
    case RVC_DISP:
        if (pSim->SS.MaxVoxDisp == 0)
            return GetJet(0);
        return GetJet(pBond->GetMaxVoxDisp() / pSim->SS.MaxVoxDisp);
        break;
    case RVC_STATE:
        if (pBond->IsBroken())
            return CColor(1.0f, 0.0f, 0.0f, 1.0f);
        else if (pBond->IsYielded())
            return CColor(1.0f, 1.0f, 0.0f, 1.0f);
        else
            return CColor(1.0f, 1.0f, 1.0f, 1.0f);
        break;
    case RVC_STRAIN_EN:
        if (pSim->SS.MaxBondStrainE == 0)
            return GetJet(0);
        return GetJet(pBond->GetStrainEnergy() / pSim->SS.MaxBondStrainE);
        break;
    case RVC_STRAIN:
        if (pSim->SS.MaxBondStrain == 0)
            return GetJet(0);
        return GetJet(pBond->GetEngStrain() / pSim->SS.MaxBondStrain);
        break;
    case RVC_STRESS:
        if (pSim->SS.MaxBondStress == 0)
            return GetJet(0);
        return GetJet(pBond->GetEngStress() / pSim->SS.MaxBondStress);
        break;
    case RVC_PRESSURE:
        return GetJet(0); // for now. Pressure of a bond doesn't really make sense
        break;
    default:
        return CColor(0.0f, 0.0f, 0.0f, 1.0f);
        break;
    }
}

CColor CVXS_SimGLView::GetCollisionBondColor(CVXS_BondCollision *pBond) {
    if (!pSim->IsFeatureEnabled(VXSFEAT_COLLISIONS))
        return CColor(0.0, 0.0, 0.0, 0.0);      // Hide me
    vfloat Force = pBond->GetForce1().Length(); // check which force to use!
    if (Force == 0.0)
        return CColor(0.3, 0.3, 1.0, 1.0);
    else
        return CColor(1.0, 0.0, 0.0, 1.0);
}
#endif

void CVXS_SimGLView::DrawSurfMesh(int Selected) {
    // if simulation has a mesh, draw it...

    // otherwise
    SmoothMesh.UpdateMesh(Selected); // updates the generated mesh
    SmoothMesh.Draw();
}

void CVXS_SimGLView::DrawVoxMesh(int Selected) {
    switch (CurViewCol) {
    case RVC_TYPE:
        defMeshVx2->updateMesh(CVX_MeshRender::MATERIAL);
        break;
    case RVC_KINETIC_EN:
        defMeshVx2->updateMesh(CVX_MeshRender::STATE_INFO, CVoxelyze::KINETIC_ENERGY);
        break;
    case RVC_DISP:
        defMeshVx2->updateMesh(CVX_MeshRender::STATE_INFO, CVoxelyze::DISPLACEMENT);
        break;
    case RVC_STATE:
        defMeshVx2->updateMesh(CVX_MeshRender::FAILURE);
        break;
    case RVC_STRAIN_EN:
        defMeshVx2->updateMesh(CVX_MeshRender::STATE_INFO, CVoxelyze::STRAIN_ENERGY);
        break;
    case RVC_STRAIN:
        defMeshVx2->updateMesh(CVX_MeshRender::STATE_INFO, CVoxelyze::ENG_STRAIN);
        break;
    case RVC_STRESS:
        defMeshVx2->updateMesh(CVX_MeshRender::STATE_INFO, CVoxelyze::ENG_STRESS);
        break;
    case RVC_PRESSURE:
        defMeshVx2->updateMesh(CVX_MeshRender::STATE_INFO, CVoxelyze::PRESSURE);
        break;
    default:
        defMeshVx2->updateMesh();
        break;
    }

    defMeshVx2->glDraw();
    //	VoxMesh.UpdateMesh(Selected); //updates the generated mesh
    //	VoxMesh.Draw();
}

void CVXS_SimGLView::DrawBonds(void) {
    float PrevLineWidth;
    glGetFloatv(GL_LINE_WIDTH, &PrevLineWidth);
    glLineWidth(3.0);
    glDisable(GL_LIGHTING);

#ifdef VX2
    CVoxelyze *pVx = &pSim->Vx;
    const std::vector<CVX_Link *> *links = pVx->linkList();
    for (std::vector<CVX_Link *>::const_iterator it = links->begin(); it != links->end(); it++) {
        CVX_Link *pLink = *it;

        Vec3D<float> P1 = (Vec3D<float>)pLink->voxel(false)->position();
        Vec3D<float> P2 = (Vec3D<float>)pLink->voxel(true)->position();
        Vec3D<float> Axis;
        float AngleAmt;
        int NumSegs = 12; // number segments for smooth bonds

        CColor ThisColor = GetInternalBondColor(pLink);
        //		if (pLink->isSmallAngle()) ThisColor	=
        // CColor(0.3,0.7,0.3,1.0); //GetInternalBondColor(pBond);
        // else ThisColor = CColor(0,0,0,1.0); //GetInternalBondColor(pBond);
        glColor4f(ThisColor.r, ThisColor.g, ThisColor.b, ThisColor.a);

        if (CurViewVox == RVV_SMOOTH) {
            Quat3D<float> A1 = (Quat3D<float>)pLink->voxel(false)->orientation();
            Quat3D<float> A2 = (Quat3D<float>)pLink->voxel(true)->orientation();
            A1.AngleAxis(AngleAmt, Axis); // get angle/axis for A1

            Vec3D<float> Pos2L = A1.RotateVec3DInv(P2 - P1); // Get PosDif in local coordinate system
            Quat3D<float> Angle2L = A2 * A1.Conjugate();     // rotate A2 by A1
            pLink->toAxisX(&Pos2L);                          // swing bonds in the +Y and +Z directions to +X
            pLink->toAxisX(&Angle2L);

            vfloat L = Pos2L.x; // pV1->GetNominalSize();
            Vec3D<float> Angle2LV = Angle2L.ToRotationVector();
            vfloat ay = (Angle2LV.z * L - 2 * Pos2L.y) / (L * L * L);
            vfloat by = (3 * Pos2L.y - Angle2LV.z * L) / (L * L);
            vfloat az = (-Angle2LV.y * L - 2 * Pos2L.z) / (L * L * L);
            vfloat bz = (3 * Pos2L.z + Angle2LV.y * L) / (L * L);

            glPushMatrix();
            glTranslated(P1.x, P1.y, P1.z);
            glRotated(AngleAmt * 180 / 3.1415926, Axis.x, Axis.y,
                      Axis.z); // rotate to voxel 1's coordinate system
            glBegin(GL_LINE_STRIP);
            glLoadName(-1); // to disable picking

            for (int i = 0; i <= NumSegs; i++) {
                vfloat iL = ((float)i) / NumSegs * L;
                Vec3D<float> ThisPoint = Vec3D<>(iL, ay * iL * iL * iL + by * iL * iL, az * iL * iL * iL + bz * iL * iL);
                pLink->toAxisOriginal(&ThisPoint);
                glVertex3d(ThisPoint.x, ThisPoint.y, ThisPoint.z);
            }

            glEnd();
            glPopMatrix();
        } else {
            glBegin(GL_LINES);
            glLoadName(-1); // to disable picking
            if (ThisColor.a != 0.0) {
                glVertex3f((float)P1.x, (float)P1.y, (float)P1.z);
                glVertex3f((float)P2.x, (float)P2.y, (float)P2.z);
            }
            glEnd();
        }
    }

    const std::vector<CVX_Collision *> cols = *pVx->collisionList();

    glBegin(GL_LINES);
    glLoadName(-1); // to disable picking
    for (std::vector<CVX_Collision *>::const_iterator it = cols.begin(); it != cols.end(); it++) {
        //{
        // const CVX_Voxel* pV1 = (*it);
        // const std::list<CVX_Voxel*>* colVoxels = pV1->pColWatchList;

        // for (std::list<CVX_Voxel*>::const_iterator jt =colVoxels->begin();
        // jt!=colVoxels->end(); jt++){

        //	CVX_Voxel* pV2 = (*jt);

        CColor ThisColor(0, 0,
                         255); // = GetCollisionBondColor(&pSim->BondArrayCollision[i]);
        Vec3D<> P1 = (*it)->voxel1()->pos;
        Vec3D<> P2 = (*it)->voxel2()->pos;

        glColor4f(ThisColor.r, ThisColor.g, ThisColor.b, ThisColor.a);
        if (ThisColor.a != 0.0) {
            glVertex3f((float)P1.x, (float)P1.y, (float)P1.z);
            glVertex3f((float)P2.x, (float)P2.y, (float)P2.z);
        }
        //	}
    }

    glEnd();
    glLineWidth(PrevLineWidth);
    glEnable(GL_LIGHTING);

#else
    //	bool DrawInputBond = true;

    Vec3D<> P1, P2, Axis;
    CVXS_Voxel *pV1, *pV2;
    vfloat AngleAmt;
    int NumSegs = 12; // number segments for smooth bonds

    int iT = pSim->NumBond();

    for (int i = 0; i < iT; i++) // go through all the bonds...
    {
        CVXS_BondInternal *pBond = &pSim->BondArrayInternal[i];
        pV1 = pBond->GetpV1();
        pV2 = pBond->GetpV2();

        // set color
        CColor ThisColor = GetInternalBondColor(pBond);
        glColor4f(ThisColor.r, ThisColor.g, ThisColor.b, ThisColor.a);

        P1 = pV1->GetCurPos();
        P2 = pV2->GetCurPos();

        if (CurViewVox == RVV_SMOOTH) {
            Quat3D<> A1 = pV1->GetCurAngle();
            Quat3D<> A2 = pV2->GetCurAngle();
            A1.AngleAxis(AngleAmt, Axis); // get angle/axis for A1

            Vec3D<> Pos2L = A1.RotateVec3DInv(P2 - P1); // Get PosDif in local coordinate system
            Quat3D<> Angle2L = A2 * A1.Conjugate();     // rotate A2 by A1
            pBond->ToXDirBond(&Pos2L);                  // swing bonds in the +Y and +Z directions to +X
            pBond->ToXDirBond(&Angle2L);

            vfloat L = Pos2L.x; // pV1->GetNominalSize();
            Vec3D<> Angle2LV = Angle2L.ToRotationVector();
            vfloat ay = (Angle2LV.z * L - 2 * Pos2L.y) / (L * L * L);
            vfloat by = (3 * Pos2L.y - Angle2LV.z * L) / (L * L);
            vfloat az = (-Angle2LV.y * L - 2 * Pos2L.z) / (L * L * L);
            vfloat bz = (3 * Pos2L.z + Angle2LV.y * L) / (L * L);

            glPushMatrix();
            glTranslated(P1.x, P1.y, P1.z);
            glRotated(AngleAmt * 180 / 3.1415926, Axis.x, Axis.y,
                      Axis.z); // rotate to voxel 1's coordinate system
            glBegin(GL_LINE_STRIP);
            glLoadName(-1); // to disable picking

            for (int i = 0; i <= NumSegs; i++) {
                vfloat iL = ((float)i) / NumSegs * L;
                Vec3D<> ThisPoint = Vec3D<>(iL, ay * iL * iL * iL + by * iL * iL, az * iL * iL * iL + bz * iL * iL);
                pBond->ToOrigDirBond(&ThisPoint);
                glVertex3d(ThisPoint.x, ThisPoint.y, ThisPoint.z);
            }

            glEnd();
            glPopMatrix();
        } else { // straight lines (faster)
            glBegin(GL_LINES);
            glLoadName(-1); // to disable picking

            if (ThisColor.a != 0.0) {
                glVertex3f((float)P1.x, (float)P1.y, (float)P1.z);
                glVertex3f((float)P2.x, (float)P2.y, (float)P2.z);
            }

            glEnd();
        }
    }

    iT = pSim->NumColBond();
    glBegin(GL_LINES);
    glLoadName(-1);              // to disable picking
    for (int i = 0; i < iT; i++) // go through all the bonds...
    {
        pV1 = pSim->BondArrayCollision[i].GetpV1();
        pV2 = pSim->BondArrayCollision[i].GetpV2();

        CColor ThisColor = GetCollisionBondColor(&pSim->BondArrayCollision[i]);
        P1 = pV1->GetCurPos();
        P2 = pV2->GetCurPos();

        glColor4f(ThisColor.r, ThisColor.g, ThisColor.b, ThisColor.a);
        if (ThisColor.a != 0.0) {
            glVertex3f((float)P1.x, (float)P1.y, (float)P1.z);
            glVertex3f((float)P2.x, (float)P2.y, (float)P2.z);
        }
    }

    ////input bond
    // if (DrawInputBond && BondInput->GetpV1() && BondInput->GetpV2()){
    //	glColor4f(1.0, 0, 0, 1.0);
    //	P1 = BondInput->GetpV1()->GetCurPos();
    //	P2 = BondInput->GetpV2()->GetCurPos();
    //	glVertex3f((float)P1.x, (float)P1.y, (float)P1.z);
    // glVertex3f((float)P2.x, (float)P2.y, (float)P2.z);
    //}

    glEnd();

#endif

    glLineWidth(PrevLineWidth);
    glEnable(GL_LIGHTING);
}
//
// void CVXS_SimGLView::DrawMiniVoxels() //draws grab-able mini voxels with
// space to show bonds or forces
//{
//	Vec3D<> Center;
//	iT = pSim->NumVox();
//	glPointSize(5.0);
//	Vec3D<> tmp(0,0,0);
//	for (int i = 0; i<iT; i++) //go through all the voxels...
//	{
//		//mostly copied from Voxel drawing function!
//		Center = pSim->VoxArray[i].GetCurPos();
//		glColor4d(0.2, 0.2, 0.2, 1.0);
//	//	glLoadName (StoXIndexMap[i]); //to enable picking
//
//		glPushMatrix();
//		glTranslated(Center.x, Center.y, Center.z);
//		glLoadName (pSim->StoXIndexMap[i]); //to enable picking
//
//		//generate rotation matrix here!!! (from quaternion)
//		Vec3D<> Axis;
//		vfloat AngleAmt;
//		Quat3D<>(pSim->VoxArray[i].GetCurAngle()).AngleAxis(AngleAmt,
// Axis); 		glRotated(AngleAmt*180/3.1415926, Axis.x, Axis.y,
// Axis.z);
//
//		vfloat Scale = pSim->VoxArray[i].GetCurScale(); //show deformed
// voxel size 		glScaled(Scale, Scale, Scale);
//
//		//LocalVXC.Voxel.DrawVoxel(&tmp,
// LocalVXC.Lattice.Lattice_Dim*(1+0.5*CurTemp *
// pMaterials[CVoxelArray[i].MatIndex].CTE), LocalVXC.Lattice.Z_Dim_Adj);
//		pSim->LocalVXC.Voxel.DrawVoxel(&tmp, 0.2);
////LocalVXC.GetLatticeDim()); //[i].CurSize.x); //,
// LocalVXC.Lattice.Z_Dim_Adj);
//
//		glPopMatrix();
//	}
//
//
//	glLineWidth(PrevLineWidth);
//	glEnable(GL_LIGHTING);
//}

void CVXS_SimGLView::DrawAngles(void) {
#ifndef VX2
    // draw directions
    float PrevLineWidth;
    glGetFloatv(GL_LINE_WIDTH, &PrevLineWidth);
    glLineWidth(2.0);
    glDisable(GL_LIGHTING);

    glBegin(GL_LINES);

    for (int i = 0; i < pSim->NumVox(); i++) { // go through all the voxels... (GOOD FOR ONLY SMALL
                                               // DISPLACEMENTS, I THINK... think through transformations
                                               // here!)
        glColor3f(1, 0, 0);                    //+X direction
        glVertex3d(pSim->VoxArray[i].GetCurPos().x, pSim->VoxArray[i].GetCurPos().y, pSim->VoxArray[i].GetCurPos().z);
        Vec3D<> Axis1(pSim->LocalVXC.GetLatticeDim() / 4, 0, 0);
        Vec3D<> RotAxis1 = pSim->VoxArray[i].GetCurAngle().RotateVec3D(Axis1);
        //	Vec3D<> RotAxis1 =
        //(pSim->VoxArray[i].GetCurAngle()*Quat3D<>(Axis1)*pSim->VoxArray[i].GetCurAngle().Conjugate()).ToVec();
        glVertex3d(pSim->VoxArray[i].GetCurPos().x + RotAxis1.x, pSim->VoxArray[i].GetCurPos().y + RotAxis1.y,
                   pSim->VoxArray[i].GetCurPos().z + RotAxis1.z);

        glColor3f(0, 1, 0); //+Y direction
        glVertex3d(pSim->VoxArray[i].GetCurPos().x, pSim->VoxArray[i].GetCurPos().y, pSim->VoxArray[i].GetCurPos().z);
        Axis1 = Vec3D<>(0, pSim->LocalVXC.GetLatticeDim() / 4, 0);
        RotAxis1 = pSim->VoxArray[i].GetCurAngle().RotateVec3D(Axis1);
        //		RotAxis1 =
        //(pSim->VoxArray[i].GetCurAngle()*Quat3D<>(Axis1)*pSim->VoxArray[i].GetCurAngle().Conjugate()).ToVec();
        glVertex3d(pSim->VoxArray[i].GetCurPos().x + RotAxis1.x, pSim->VoxArray[i].GetCurPos().y + RotAxis1.y,
                   pSim->VoxArray[i].GetCurPos().z + RotAxis1.z);

        glColor3f(0, 0, 1); //+Z direction
        glVertex3d(pSim->VoxArray[i].GetCurPos().x, pSim->VoxArray[i].GetCurPos().y, pSim->VoxArray[i].GetCurPos().z);
        Axis1 = Vec3D<>(0, 0, pSim->LocalVXC.GetLatticeDim() / 4);
        RotAxis1 = pSim->VoxArray[i].GetCurAngle().RotateVec3D(Axis1);
        //		RotAxis1 =
        //(pSim->VoxArray[i].GetCurAngle()*Quat3D<>(Axis1)*pSim->VoxArray[i].GetCurAngle().Conjugate()).ToVec();
        glVertex3d(pSim->VoxArray[i].GetCurPos().x + RotAxis1.x, pSim->VoxArray[i].GetCurPos().y + RotAxis1.y,
                   pSim->VoxArray[i].GetCurPos().z + RotAxis1.z);
    }
    glEnd();

    glLineWidth(PrevLineWidth);
    glEnable(GL_LIGHTING);
#endif
}

void CVXS_SimGLView::DrawStaticFric(void) {
#ifdef VX2
    // draw triangle for points that are stuck via static friction
    CVoxelyze *pVx = &pSim->Vx;

    glBegin(GL_TRIANGLES);
    glColor4f(255, 255, 0, 1.0);
    vfloat dist = pVx->voxelSize() / 3;
    const std::vector<CVX_Voxel *> *pVl = pVx->voxelList();

    int iT = pVl->size();
    Vec3D<> P1;
    for (int i = 0; i < iT; i++) {                  // go through all the voxels...
        if (((*pVl)[i])->isFloorStaticFriction()) { // draw point if static friction...
            P1 = (*pVl)[i]->position();             // pSim->VoxArray[i].GetCurPos();
            glVertex3f((float)P1.x, (float)P1.y, (float)P1.z);
            glVertex3f((float)P1.x, (float)(P1.y - dist / 2), (float)(P1.z + dist));
            glVertex3f((float)P1.x, (float)(P1.y + dist / 2), (float)(P1.z + dist));
        }
    }
    glEnd();
#else
    // draw triangle for points that are stuck via static friction
    glBegin(GL_TRIANGLES);
    glColor4f(255, 255, 0, 1.0);
    vfloat dist = pSim->VoxArray[0].GetNominalSize() / 3; // needs work!!
    int iT = pSim->NumVox();
    Vec3D<> P1;
    for (int i = 0; i < iT; i++) {                  // go through all the voxels...
        if (pSim->VoxArray[i].GetCurStaticFric()) { // draw point if static friction...
            P1 = pSim->VoxArray[i].GetCurPos();
            glVertex3f((float)P1.x, (float)P1.y, (float)P1.z);
            glVertex3f((float)P1.x, (float)(P1.y - dist / 2), (float)(P1.z + dist));
            glVertex3f((float)P1.x, (float)(P1.y + dist / 2), (float)(P1.z + dist));
        }
    }
    glEnd();
#endif
}

int CVXS_SimGLView::StatRqdToDraw() // returns the stats bitfield that we need
                                    // to calculate to draw the current view.
{
    if (CurViewMode == RVM_NONE)
        return CALCSTAT_NONE;
    switch (CurViewCol) {
    case RVC_KINETIC_EN:
        return CALCSTAT_KINE;
        break;
    case RVC_DISP:
        return CALCSTAT_DISP;
        break;
    case RVC_STRAIN_EN:
        return CALCSTAT_STRAINE;
        break;
    case RVC_STRAIN:
        return CALCSTAT_ENGSTRAIN;
        break;
    case RVC_STRESS:
        return CALCSTAT_ENGSTRESS;
        break;
    case RVC_PRESSURE:
        return CALCSTAT_PRESSURE;
        break;
    default:
        return CALCSTAT_NONE;
    }
}

// Read a .history file and draw in an loop.
void CVXS_SimGLView::DrawHistory(int Selected, ViewVoxel historyView) {
    if (drawCylinder) {
        Vec3D<> v1(cylinderX, cylinderY, 0.01);
        Vec3D<> v2(cylinderX, cylinderY, -0.005); // floor is actually at -voxelSize/2
        CColor c(1.0, 0, 0, 0.5);
        CGL_Utils::DrawCylinder(v1, v2, cylinderR, Vec3D<>(1.0, 1.0, 1.0), true, false, 1.0, c);
    }
    if (drawRectangle) {
        CColor c(1.0, 0, 0, 0.5);
        Vec3D<> v1(rectangleX - rectangleA, rectangleY - rectangleB, 0);
        Vec3D<> v2(rectangleX + rectangleA, rectangleY - rectangleB, 0.01);
        Vec3D<> v3(rectangleX + rectangleA, rectangleY + rectangleB, 0);
        Vec3D<> v4(rectangleX - rectangleA, rectangleY + rectangleB, 0.01);
        CGL_Utils::DrawRectangle(v1, v2, true, 0, c);
        CGL_Utils::DrawRectangle(v2, v3, true, 0, c);
        CGL_Utils::DrawRectangle(v3, v4, true, 0, c);
        CGL_Utils::DrawRectangle(v4, v1, true, 0, c);
    }
    QStringList pos;
    double voltage;
    CColor voltageColor;
    CColor defaultColor(0.2, 0.2, 0.2, 0.2);
    CColor pickedColor(0.2,1.0,0.2,0.2);
    CColor colorMap[10];
    colorMap[0] = CColor(0.9f, 0.2f, 0.29f, 0.5f);
    colorMap[1] = CColor(0.6f, 0.6f, 0.5f, 0.5f);
    colorMap[2] = CColor(0.85f, 0.75f, 0.24f, 0.8f);
    colorMap[3] = CColor(0.41f, 0.73f, 0.49f, 0.5f);
    colorMap[4] = CColor(0.30f, 0.70f, 0.37f, 0.5f);
    colorMap[5] = CColor(0.75f, 0.75f, 0.50f, 0.8f);
    colorMap[6] = CColor(0.35f, 0.56f, 0.39f, 0.5f);
    colorMap[7] = CColor(0.82f, 0.61f, 0.49f, 0.5f);
    colorMap[8] = CColor(0.85f, 0.76f, 0.39f, 0.5f);
    colorMap[9] = CColor(0.97f, 0.55f, 0.19f, 1.0f);
    if (drawFlash) {
        if (historyTime > flashT && historyTime < flashT + 0.02) {
            colorMap[2] = CColor(1.0, 0, 0, 0.5);
        }
    }
    bool ZoomNear = false;
    bool ZoomVeryFar = false;
    // printf("pGLWin->m_Cam.Zoom: %f\n", pGLWin->m_Cam.Zoom);
    if (pGLWin->m_Cam.Zoom < 50) {
        ZoomNear = true;
    }
    if (pGLWin->m_Cam.Zoom > 400) {
        ZoomVeryFar = true;
    }
    // If it doesn't start play, check the Default.vxc file
    if (pSim->StreamHistory) {
        while (1) {
            if (pSim->StreamHistory->atEnd()) {
                qInfo() << "history loop, reload the file.";
                pSim->LoadHistoryFile(pSim->fileHistory->fileName());
                // pSim->StreamHistory->seek(0);
            }
            QString line;
            if (HistoryPaused && currentHistoryLine != "") {
                line = currentHistoryLine;
            } else {
                line = pSim->StreamHistory->readLine();
            }
            if (!line.isNull()) {
                QString token_setting = "{{{setting}}}";
                if (line.indexOf(token_setting) != -1) {
                    QString str_setting = line.mid(token_setting.length(), line.length() - 1);
                    // qInfo() << str_setting;
                    std::istringstream is(str_setting.toStdString());

                    pt::ptree tree;
                    pt::read_xml(is, tree);
                    auto cylinder = tree.get_child_optional("cylinder");
                    if (cylinder) {
                        drawCylinder = true;
                        cylinderX = tree.get("cylinder.x", 0.0);
                        cylinderY = tree.get("cylinder.y", 0.0);
                        cylinderR = tree.get("cylinder.r", 1.0);
                        continue;
                    }
                    auto rectangle = tree.get_child_optional("rectangle");
                    if (rectangle) {
                        drawRectangle = true;
                        rectangleX = tree.get("rectangle.x", 0.0);
                        rectangleY = tree.get("rectangle.y", 0.0);
                        rectangleA = tree.get("rectangle.a", 0.0);
                        rectangleB = tree.get("rectangle.b", 0.0);
                        continue;
                    }
                    auto flash = tree.get_child_optional("flash");
                    if (flash) {
                        drawFlash = true;
                        flashT = tree.get("flash.t", 0.0);
                        continue;
                    }
                    auto matid = tree.get_child_optional("matcolor.id");
                    if (matid) {
                        auto id = tree.get<int>("matcolor.id", 0);
                        auto r = tree.get<double>("matcolor.r", 0);
                        auto g = tree.get<double>("matcolor.g", 0);
                        auto b = tree.get<double>("matcolor.b", 0);
                        auto a = tree.get<double>("matcolor.a", 0);
                        matColors[id] = CColor(r, g, b, a);
                        continue;
                    }
                    // rescale the whole space. so history file can contain less digits. ( e.g. not 0.000221, but 2.21 )
                    auto op_rescale = tree.get_child_optional("rescale");
                    if (op_rescale) {
                        rescale = tree.get("rescale", 1.0);
                    }
                    // voxel size
                    auto op_voxel_size = tree.get_child_optional("voxel_size");
                    if (op_voxel_size) {
                        voxel_size = tree.get("voxel_size", 0.01);
                    }
                }
                int j = 0;
                QStringList voxel_link = line.split("|");
                if (voxel_link.size() > 1) {
                    if ((j = voxel_link[1].indexOf("]]]")) != -1) {
                        glBegin(GL_LINE_STRIP);
                        float PrevLineWidth;
                        float x1, y1, z1, x2, y2, z2;
                        glColor4f(colorMap[9].r, colorMap[9].g, colorMap[9].b, colorMap[9].a);
                        glGetFloatv(GL_LINE_WIDTH, &PrevLineWidth);
                        glLineWidth(3.0);
                        glDisable(GL_LIGHTING);

                        QString mline = voxel_link[1].mid(j + 3, voxel_link[1].length() - j - 10);
                        QStringList link = mline.split(";");
                        for (int i = 0; i < link.size(); i++) {
                            pos = link[i].split(",");
                            if (pos.size() <= 1)
                                continue;
                            if (pos.size() < 6) {
                                qWarning() << "ERROR: a link has pos size is " << pos.size() << "<6." << link[i];
                                continue;
                            }
                            x1 = pos[0].toDouble();
                            y1 = pos[1].toDouble();
                            z1 = pos[2].toDouble();
                            x2 = pos[3].toDouble();
                            y2 = pos[4].toDouble();
                            z2 = pos[5].toDouble();
                            glBegin(GL_LINES);
                            glLoadName(-1); // to disable picking
                            glVertex3f(x1, y1, z1);
                            glVertex3f(x2, y2, z2);
                            glEnd();
                        }
                        glEnd();
                        glLineWidth(PrevLineWidth);
                        glEnable(GL_LIGHTING);
                    }
                }
                if ((j = voxel_link[0].indexOf(">>>")) != -1) {
                    Message = voxel_link[0].mid(3, j - 3);
                    int k = voxel_link[0].indexOf("Time:");
                    historyTime = voxel_link[0].mid(k + 5, j - k - 5).toDouble();

                    QString mline = voxel_link[0].mid(j + 3, voxel_link[0].length() - j - 10);
                    QStringList voxel = mline.split(";");
                    Message += "\n\nVoxels: " + QString::number(voxel.size()) + "\n";
                    Vec3D<> v1, v2;
                    double p1, p2, p3;
                    double angle, r1, r2, r3;
                    int matid;
                    int i = 0;
                    int indexCounter = 0;

                    for (auto &v : voxel) {
                        // for (int i = 0; i < voxel.size(); i++) {
                        pos = v.split(",");
                        if (pos.size() <= 1)
                            continue;
                        if (pos.size() < 15) {
                            qWarning() << "ERROR: a voxel has pos size is " << pos.size() << "<14." << v;
                            continue;
                        }
                        glPushMatrix();
                        QStringList::const_iterator constIterator;
                        constIterator = pos.constBegin();
                        p1 = (*constIterator).toDouble() * rescale;
                        constIterator++;
                        p2 = (*constIterator).toDouble() * rescale;
                        constIterator++;
                        p3 = (*constIterator).toDouble() * rescale;
                        constIterator++;
                        angle = (*constIterator).toDouble();
                        constIterator++;
                        r1 = (*constIterator).toDouble();
                        constIterator++;
                        r2 = (*constIterator).toDouble();
                        constIterator++;
                        r3 = (*constIterator).toDouble();
                        constIterator++;
                        Vec3D<double> nnn, ppp;
                        nnn.x = (*constIterator).toDouble() * rescale;
                        constIterator++;
                        nnn.y = (*constIterator).toDouble() * rescale;
                        constIterator++;
                        nnn.z = (*constIterator).toDouble() * rescale;
                        constIterator++;
                        ppp.x = (*constIterator).toDouble() * rescale;
                        constIterator++;
                        ppp.y = (*constIterator).toDouble() * rescale;
                        constIterator++;
                        ppp.z = (*constIterator).toDouble() * rescale;
                        constIterator++;
                        matid = (*constIterator).toInt();
                        if (matid < 0 || matid >= 10) {
                            matid = 0;
                        }
                        constIterator++;
                        voltage = (*constIterator).toDouble();
                        glLoadName(++indexCounter); // to enable picking
                        if (Selected==indexCounter && !HistoryPaused) { //add a new point to the plot
                            emit ReqUpdateVoltagePlot(historyTime, voltage, indexCounter);
                        }
                        glTranslated(p1, p2, p3);
                        glRotated(angle, r1, r2, r3);
                        if (nnn.Dist2(ppp) < 1) {
                            CColor c;
                            if (Selected==indexCounter) {
                                c = pickedColor;
                            } else {
                                if (historyView==RVV_HISTORY_ELECTRICAL) {
                                    voltageColor = GetJet(voltage/100.0);
                                    c = voltageColor;
                                } else if (historyView==RVV_HISTORY_ROTATION) {
                                    //c = matColors[matid];
                                    c = CColor(0,0,0,0.8);
                                    c.b = angle/60;
                                    if (c.b>1) c.b=1;
                                    c.r = 1-c.b;
                                    c.g = (1-c.b)*0.5;
                                    // if (indexCounter==1) {
                                        // printf("%.02f, %.02f, %.02f, %.02f = %.02f\n", angle, r1, r2, r3, c.b);
                                    // }
                                    // c = GetRnB
                                } else {
                                    if (matColors.find(matid) != matColors.end()) {
                                        c = matColors[matid];
                                    } else {
                                        printf("Color not found %d.\n", matid);
                                        c = defaultColor;
                                    }
                                }
                            }
                            CGL_Utils::DrawCube(nnn, ppp, true, true, 1.0, c, false);
                            // CGL_Utils::DrawCube(nnn, ppp, true, ZoomNear, 1.0, c, ZoomVeryFar);
                        }
                        glPopMatrix();
                        // Update camera view center, but gentlely.
                        if (i++ == int(voxel.size() / 2))
                            HistoryCM = HistoryCM * 0.95 + Vec3D<>(p1, p2, p3) * 0.05;
                    }
                    currentHistoryLine = line;
                    return;
                }
            }
        }
    }
}
CColor CVXS_SimGLView::GetRnB(double v) {
    if (v > 1)
        v = 1;
    else if (v < 0)
        v = 0;
    return CColor(v, 1.5 * (1 - v) * v, (1 - v), 0.6);
}