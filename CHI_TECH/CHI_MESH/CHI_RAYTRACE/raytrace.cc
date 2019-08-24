#include "../chi_mesh.h"

#include <CHI_MESH/CHI_MESHCONTINUUM/chi_meshcontinuum.h>
#include <CHI_MESH/CHI_CELL/cell_slab.h>
#include <CHI_MESH/CHI_CELL/cell_polygon.h>
#include <CHI_MESH/CHI_CELL/cell_polyhedron.h>

#include <chi_log.h>

extern CHI_LOG chi_log;

//###################################################################
/**Performs raytracing on chi_mesh type cells. With the exception
 * of slab-type cells this algorithm essentially looks for
 * intersection with triangles. The first step involves check
 * the intersection with a plane formed by a given triangle's
 * normal and a reference point.
 *
 * The pointer to aux_info must be a data structure with the first
 * element being an integer.
 *
 * \param Reference to the grid on which the specific cell resides.
 * \param A pointer to the cell base.
 * \param The initial position of the ray.
 * \param The direction of the ray.
 * \param A reference to a variable that will receive the distance to the
 *        surface.
 * \param A reference to the vector to receive the final position.
 * \param (Optional) A pointer to an integer array of minimum size 1. The
 *        first integer in this array indicates how many elements there are in
 *        the array. If the array size is 2, the seconds element (i.e. element[1]
 *        will receive the associated face of the surface of the cell. If
 *        the array size is 3 then the third element will be the cell/bndry
 *        on the either side of the face.*/
void chi_mesh::RayTrace(chi_mesh::MeshContinuum* grid,
                        chi_mesh::Cell *cell,
                        const chi_mesh::Vector &pos_i,
                        const chi_mesh::Vector &omega_i,
                        double& d_to_surface,
                        chi_mesh::Vector &pos_f,
                        int *aux_info)
{

  //We make these copies so we can do arithmetic on them
  chi_mesh::Vector pos_i_copy = pos_i;
  chi_mesh::Vector omega_i_copy = omega_i;

  //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ SLAB
  if (typeid(*cell) == typeid(chi_mesh::CellSlab))
  {
    auto slab_cell = (chi_mesh::CellSlab*)cell;

    chi_mesh::Vector intersection_point;
    std::pair<double,double> weights;

    int num_faces = 2;
    for (int f=0; f<num_faces; f++)
    {
      int fpi = slab_cell->v_indices[f]; //face point index
      chi_mesh::Vertex face_point = *grid->nodes[fpi];

      double extention_distance = 2.0*(*grid->nodes[0]-*grid->nodes[1]).Norm();
      extention_distance = 1.0e15;

      bool intersects = chi_mesh::CheckPlaneLineIntersect(
                        slab_cell->face_normals[f],
                        face_point,
                        pos_i,
                        pos_i_copy + omega_i_copy*extention_distance,
                        intersection_point,
                        weights);

      double D = weights.first*extention_distance;

      if ( ((D) > 1.0e-10) and (intersects) )
      {
        d_to_surface = D;
        pos_f = pos_i_copy + omega_i_copy*D;

        if (aux_info != nullptr)
        {
          if (aux_info[0] >= 2)
            aux_info[1] = f;
          if (aux_info[0] >= 3)
            aux_info[2] = slab_cell->edges[f];
        }
        break;
      }
    }


  }//slab
  else
  {
    chi_log.Log(LOG_ALLERROR)
      << "Unsupported cell type encountered in call to "
      << "chi_mesh::RayTrace.";
    exit(EXIT_FAILURE);
  }

}