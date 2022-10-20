#ifndef TH_ARRAY_H
#define TH_ARRAY_H

// The whole reason I switched to c++ :)
template <typename T, uint32 max_count_in>
struct array_flat {
	uint32 count;
	uint32 max_count = max_count_in;
	T elements[max_count_in];

	T& operator[](uint32 index) {
		assert(index >= 0 && index < max_count);
		return elements[index];
	}

	// Pushes a new element at end of array. Sets hell loose when it reaches the max
	T* push(const T& element) {
		assert(count + 1 < max_count);
		T* new_element = &elements[count++];
		*new_element = element;
		return new_element;
	}

	// overcomplex as fuck.
	// this is why c++ is fucking stupid
	// knock knock knockin on complexities dooooooooooooor
	/*
	// Push new element that wraps around to the start of the array when max_count is reached.
	T* push_circular(const T& element) {
		if (count + 1 < max_count) {
			return push(element);
		}
		else {
			(circular_index + 1 < max_count) ? circular_index++ : circular_index = 0;
			T* new_element = &elements[circular_index];
			*new_element = element;
			return new_element;
		}
	}

	void destroy(const uint32 index) {
		assert(index >= 0 && index < max_count);
		assert(valid[index]); // already destroyed?
		MEMORY_ZERO_STRUCT(&elements[index]);
		valid[index] = 0;
	}
	*/
};

#endif