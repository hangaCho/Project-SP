/* This file completely derives from IRRLICHT SDK */
#include <new>
#define DEBUG_BREAK(_CONDITION_) if (_CONDITION_) {_asm int 3}
enum eAllocStrategy
{
	ALLOC_STRATEGY_SAFE    = 0,
	ALLOC_STRATEGY_DOUBLE  = 1,
	ALLOC_STRATEGY_SQRT    = 2
};
template<typename T>
class Allocator
{
public:
	virtual ~Allocator() {}
	T* allocate(size_t cnt)
	{
		return (T*)internal_new(cnt* sizeof(T));
	}
	void deallocate(T* ptr)
	{
		internal_delete(ptr);
	}
	void construct(T* ptr, const T&e)
	{
		new ((void*)ptr) T(e);
	}
	void destruct(T* ptr)
	{
		ptr->~T();
	}
protected:
	virtual void* internal_new(size_t cnt)
	{
		return operator new(cnt);
	}
	virtual void internal_delete(void* ptr)
	{
		operator delete(ptr);
	}
};
template <class T, typename TAlloc = Allocator<T> >
class Arr
{
public:
	Arr()
		: data(0), allocated(0), used(0),
			strategy(ALLOC_STRATEGY_DOUBLE), free_when_destroyed(true), is_sorted(true)
	{}
	Arr(unsigned int start_count)
      : data(0), allocated(0), used(0),
        strategy(ALLOC_STRATEGY_DOUBLE), free_when_destroyed(true), is_sorted(true)
	{
		reallocate(start_count);
	}
	Arr(const Arr<T, TAlloc>& other) : data(0)
	{
		*this = other;
	}
	~Arr()
	{
		clear();
	}
	void reallocate(unsigned int new_size)
	{
		T* old_data = data;

		data = allocator.allocate(new_size);
		allocated = new_size;
		int end = used < new_size ? used : new_size;
		for (int i=0; i<end; ++i)
		{
			allocator.construct(&data[i], old_data[i]);
		}
		for (unsigned int j=0; j<used; ++j)
			allocator.destruct(&old_data[j]);
		if (allocated < used)
			used = allocated;
		allocator.deallocate(old_data);
	}
	void setAllocStrategy ( eAllocStrategy newStrategy = ALLOC_STRATEGY_DOUBLE )
	{
		strategy = newStrategy;
	}
	void push_back(const T& element)
	{
		insert(element, used);
	}
	void push_front(const T& element)
	{
		insert(element);
	}
	void insert(const T& element, unsigned int index=0)
	{
		assert(index<=used);
		if (used + 1 > allocated)
		{
			const T e(element);
			unsigned int newAlloc;
			switch ( strategy )
			{
				case ALLOC_STRATEGY_DOUBLE:
					newAlloc = used + 1 + (allocated < 500 ?
							(allocated < 5 ? 5 : used) : used >> 2);
					break;
				default:
				case ALLOC_STRATEGY_SAFE:
					newAlloc = used + 1;
					break;
			}
			reallocate( newAlloc);
			for (unsigned int i=used; i>index; --i)
			{
				if (i<used)
					allocator.destruct(&data[i]);
				allocator.construct(&data[i], data[i-1]);
			}
			if (used > index)
				allocator.destruct(&data[index]);
			allocator.construct(&data[index], e);
		}
		else
		{
			if ( used > index )
			{
				allocator.construct(&data[used], data[used-1]);
				for (unsigned int i=used-1; i>index; --i)
				{
					data[i] = data[i-1];
				}
				data[index] = element;
			}
			else
			{
				allocator.construct(&data[index], element);
			}
		}
		is_sorted = false;
		++used;
	}
	void clear()
	{
		if (free_when_destroyed)
		{
			for (unsigned int i=0; i<used; ++i)
				allocator.destruct(&data[i]);

			allocator.deallocate(data);
		}
		data = 0;
		used = 0;
		allocated = 0;
		is_sorted = true;
	}
	void set_pointer(T* newPointer, unsigned int size, bool _is_sorted=false, bool _free_when_destroyed=true)
	{
		clear();
		data = newPointer;
		allocated = size;
		used = size;
		is_sorted = _is_sorted;
		free_when_destroyed=_free_when_destroyed;
	}
	void set_free_when_destroyed(bool f)
	{
		free_when_destroyed = f;
	}
	void set_used(unsigned int usedNow)
	{
		if (allocated < usedNow)
			reallocate(usedNow);
		used = usedNow;
	}
	const Arr<T, TAlloc>& operator=(const Arr<T, TAlloc>& other)
	{
		if (this == &other)
			return *this;
		strategy = other.strategy;
		if (data)
			clear();
		if (other.allocated == 0)
			data = 0;
		else
			data = allocator.allocate(other.allocated);
		used = other.used;
		free_when_destroyed = true;
		is_sorted = other.is_sorted;
		allocated = other.allocated;
		for (unsigned int i=0; i<other.used; ++i)
			allocator.construct(&data[i], other.data[i]);
		return *this;
	}
	bool operator == (const Arr<T, TAlloc>& other) const
	{
		if (used != other.used)
			return false;
		for (unsigned int i=0; i<other.used; ++i)
			if (data[i] != other[i])
				return false;
		return true;
	}
	bool operator != (const Arr<T, TAlloc>& other) const
	{
		return !(*this==other);
	}
	T& operator [](unsigned int index)
	{
		assert(index<used);
		return data[index];
	}
	const T& operator [](unsigned int index) const
	{
		assert(index<used)

		return data[index];
	}
	T& getLast()
	{
		assert(used)
		return data[used-1];
	}
	const T& getLast() const
	{
		assert(used)

		return data[used-1];
	}
	T* pointer()
	{
		return data;
	}
	const T* const_pointer() const
	{
		return data;
	}
	unsigned int size() const
	{
		return used;
	}
	unsigned int allocated_size() const
	{
		return allocated;
	}
	bool empty() const
	{
		return used == 0;
	}
	void sort()
	{
		if (!is_sorted && used>1)
			heapsort(data, used);
		is_sorted = true;
	}
	int binary_search(const T& element)
	{
		sort();
		return binary_search(element, 0, used-1);
	}
	int binary_search(const T& element) const
	{
		if (is_sorted)
			return binary_search(element, 0, used-1);
		else
			return linear_search(element);
	}
	int binary_search(const T& element, int left, int right) const
	{
		if (!used)
			return -1;

		int m;

		do
		{
			m = (left+right)>>1;

			if (element < data[m])
				right = m - 1;
			else
				left = m + 1;

		} while((element < data[m] || data[m] < element) && left<=right);
		if (!(element < data[m]) && !(data[m] < element))
			return m;
		return -1;
	}
	int binary_search_multi(const T& element, int &last)
	{
		sort();
		int index = binary_search(element, 0, used-1);
		if ( index < 0 )
			return index;
		last = index;
		while ( index > 0 && !(element < data[index - 1]) && !(data[index - 1] < element) )
		{
			index -= 1;
		}
		while ( last < (int) used - 1 && !(element < data[last + 1]) && !(data[last + 1] < element) )
		{
			last += 1;
		}
		return index;
	}
	int linear_search(const T& element) const
	{
		for (unsigned int i=0; i<used; ++i)
			if (element == data[i])
				return (int)i;
		return -1;
	}
	int linear_reverse_search(const T& element) const
	{
		for (int i=used-1; i>=0; --i)
			if (data[i] == element)
				return i;

		return -1;
	}
	void erase(unsigned int index)
	{
		DEBUG_BREAK(index>=used)

		for (unsigned int i=index+1; i<used; ++i)
		{
			allocator.destruct(&data[i-1]);
			allocator.construct(&data[i-1], data[i]);
		}
		allocator.destruct(&data[used-1]);
		--used;
	}
	void erase(unsigned int index, int count)
	{
		if (index>=used || count<1)
			return;
		if (index+count>used)
			count = used-index;
		unsigned int i;
		for (i=index; i<index+count; ++i)
			allocator.destruct(&data[i]);
		for (i=index+count; i<used; ++i)
		{
			if (i > index+count)
				allocator.destruct(&data[i-count]);
			allocator.construct(&data[i-count], data[i]);
			if (i >= used-count)
				allocator.destruct(&data[i]);
		}
		used-= count;
	}
	void set_sorted(bool _is_sorted)
	{
		is_sorted = _is_sorted;
	}
	void swap(Arr<T, TAlloc>& other)
	{
		core::swap(data, other.data);
		core::swap(allocated, other.allocated);
		core::swap(used, other.used);
		core::swap(allocator, other.allocator);
		eAllocStrategy helper_strategy(strategy);
		strategy = other.strategy;
		other.strategy = helper_strategy;
		bool helper_free_when_destroyed(free_when_destroyed);
		free_when_destroyed = other.free_when_destroyed;
		other.free_when_destroyed = helper_free_when_destroyed;
		bool helper_is_sorted(is_sorted);
		is_sorted = other.is_sorted;
		other.is_sorted = helper_is_sorted;
	}
private:
	T* data;
	unsigned int allocated;
	unsigned int used;
	TAlloc allocator;
	eAllocStrategy strategy:4;
	bool free_when_destroyed:1;
	bool is_sorted:1;
};