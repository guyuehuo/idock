#include "grid_map_task.hpp"

int grid_map_task(vector<array3d<float>>& grid_maps, const vector<size_t>& atom_types_to_populate, const size_t z, const scoring_function& sf, const box& b, const receptor& rec)
{
	const size_t num_atom_types_to_populate = atom_types_to_populate.size();
	vector<float> e(num_atom_types_to_populate);

	// For each probe atom of the given X dimension value.
	const size_t num_y_probes = b.num_probes[1];
	const size_t num_x_probes = b.num_probes[0];
	for (size_t y = 0; y < num_y_probes; ++y)
	for (size_t x = 0; x < num_x_probes; ++x)
	{
		// Find the possibly interacting receptor atoms via partitions.
		const array<size_t, 3> grid_index = { x, y, z };
		const vec3 probe_coords = b.grid_corner1(grid_index);
		const vector<size_t>& receptor_atoms = rec.partitions(b.partition_index(probe_coords));

		// Accumulate individual free energies for each atom types to populate.
		fill(e.begin(), e.end(), 0.0f);
		const size_t num_receptor_atoms = receptor_atoms.size();
		for (size_t l = 0; l < num_receptor_atoms; ++l)
		{
			const atom& a = rec.atoms[receptor_atoms[l]];
			if (a.is_hydrogen()) continue;
			const float r2 = distance_sqr(probe_coords, a.coord);
			if (r2 <= scoring_function::cutoff_sqr)
			{
				const size_t t1 = a.xs;
				for (size_t i = 0; i < num_atom_types_to_populate; ++i)
				{
					const size_t t2 = atom_types_to_populate[i];
					e[i] += sf.e[sf.o(sf.p(t1, t2), r2)];
				}
			}
		}

		// Save accumulated free energies into grid maps.
		for (size_t i = 0; i < num_atom_types_to_populate; ++i)
		{
			const size_t t = atom_types_to_populate[i];
			grid_maps[t](grid_index) = e[i];
		}
	}
	return 0;
}
