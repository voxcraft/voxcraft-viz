#include <iostream>
#include "Voxelyze.h"
using namespace std;

int main(int, char**) {
    CVoxelyze Vx(0.005); //5mm voxels
    CVX_Material* pMaterial = Vx.addMaterial(1000000, 1000); //A material with stiffness E=1MPa and density 1000Kg/m^3
    vector<CVX_Voxel *> voxels;
    voxels.push_back(Vx.setVoxel(pMaterial, 0, 0, 0)); //Voxel at index x=0, y=0. z=0
    voxels.push_back(Vx.setVoxel(pMaterial, 1, 0, 0)); //Voxel at index x=0, y=0. z=0
    voxels.push_back(Vx.setVoxel(pMaterial, 2, 0, 0)); //Voxel at index x=0, y=0. z=0

    voxels[2]->external()->setFixedAll(); //Fixes all 6 degrees of freedom with an external condition on Voxel 1
    voxels[0]->external()->setForce(0, 0, -1); //pulls Voxel 3 downward with 1 Newton of force.

    for (int i=0; i<100; i++) Vx.doTimeStep(); //simulate  100 timesteps.

    for (int i=0;i<voxels.size();i++) {
        Vec3D<double> p = voxels[i]->position();
        cout << "Final Position of voxels["<<i<<"]: ("<< p.x << ", "<<p.y<<", "<<", "<< p.z<<")"<<endl;
        Quat3D<double> t = voxels[i]->orientation();
        cout << "Final Orientation of voxel["<<i<<"]: (W:"<<t.w <<", x:"<<t.x<<", y:"<<t.y<<", z:"<<t.z<<") "<<endl;
    }

}
