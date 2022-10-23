#ifndef TH_ARRAY_H
#define TH_ARRAY_H

// NOTE - I can actually just use ARRAY_COUNT on a fixed buffer to get the size, instead of having to store a #define of the size to access everywhere. Holy shit, this changes everything. I'm thinking this can probably get completely removed???
// @array - ^ test this on the next episode of myth busters

// This is literally the main reason why I switched to using C++

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

// That's it.
// thx for coming to my ted talk :)

#endif