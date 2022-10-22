#ifndef TH_ARRAY_H
#define TH_ARRAY_H

// This is literally the main reason why I switched to using C++

//#define FLAT_EACH(arr) 

template <typename T, uint32 max_count_in>
struct array_flat {
	uint32 count;
	uint32 max_count = max_count_in;
	T elements[max_count_in];

	// Can now safely access without bounds checking. Sets hell loose when out of bounds.
	T& operator[](uint32 index) {
		assert(index >= 0 && index < max_count);
		return elements[index];
	}

	// Returns next available element. Sets hell loose if the max is reached.
	T* push() {
		assert(count + 1 < max_count);
		return &elements[count++];
	}
};

// That's it.
// thx for coming to my ted talk :)

#endif