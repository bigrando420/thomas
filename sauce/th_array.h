#ifndef TH_ARRAY_H
#define TH_ARRAY_H

#define TH_ARRAY_PUSH(flat_array, count) &flat_array[count++]; Assert((count) + 1 < ArrayCount(flat_array))

// lmfao
#if 0
template <typename T, uint32 max_count_in>
struct array_flat {
	uint32 count;
	uint32 max_count = max_count_in;
	T elements[max_count_in];

	// Can now safely access without bounds checking. Sets hell loose when out of bounds.
	T& operator[](uint32 index) {
		Assert(index >= 0 && index < max_count);
		return elements[index];
	}

	// Returns next available element. Sets hell loose if the max is reached.
	T* push() {
		Assert(count + 1 < max_count);
		return &elements[count++];
	}
};
#endif

#endif