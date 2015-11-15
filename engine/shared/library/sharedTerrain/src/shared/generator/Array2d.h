//===================================================================
//
// Array2d.h
// asommers 9-11-2000
//
// copyright 2000, verant interactive
//
//--
//
// Array2d abstracts a 2d array of values. When accessing an entry
// in the 2d array, if you specify an out-of-bounds entry, the coordinate
// will clamp
//
//===================================================================

#ifndef INCLUDED_Array2d_H
#define INCLUDED_Array2d_H

//===================================================================

template <class T>
class Array2d
{
private:

	int width;
	int height;

	T*  data;

private:

	void     deallocate ();

private:

	Array2d (const Array2d& rhs);
	Array2d& operator= (const Array2d& rhs);

public:

	Array2d ();
	~Array2d ();

	int      getWidth () const;
	int      getHeight () const;
	bool     isEmpty () const;
	void     allocate (int width, int height);
	void     allocateAndSet (int width, int height, const T* newData);
	void     makeZero ();
	void     makeValue (const T& newValue);
	void     makeCopy (const Array2d<T>& rhs);

	const T* getData () const;

	const T& getData (int x, int z) const;
	T&       getData (int x, int z);

	void     setData (int x, int z, const T& newData);
};

//===================================================================

template <class T>
Array2d<T>::Array2d () :
	width (0),
	height (0),
	data (0)
{
}

//-------------------------------------------------------------------

template <class T>
Array2d<T>::~Array2d ()
{
	width = height = 0;

	deallocate ();
} //lint !e1740  // pointer member data not freed

//-------------------------------------------------------------------

template <class T>
int Array2d<T>::getWidth () const
{
	return width;
}

//-------------------------------------------------------------------

template <class T>
int Array2d<T>::getHeight () const
{
	return height;
}

//-------------------------------------------------------------------

template <class T>
bool Array2d<T>::isEmpty () const
{
	return width == 0 || height == 0;
}

//-------------------------------------------------------------------

template <class T>
void Array2d<T>::deallocate ()
{
	if (data)
	{
		delete [] data;
		data = 0;
	}
}

//-------------------------------------------------------------------

template <class T>
void Array2d<T>::allocate (int newWidth, int newHeight)
{
	DEBUG_FATAL (newWidth  <= 0, ("newWidth  <= 0"));
	DEBUG_FATAL (newHeight <= 0, ("newHeight <= 0"));

	if (newWidth > width || newHeight > height)
	{
		deallocate ();

		width  = newWidth;
		height = newHeight;

		data = new T [newWidth * newHeight];  //lint !e737  //-- sizeof
	}
}

//-------------------------------------------------------------------

template <class T>
void Array2d<T>::allocateAndSet (int newWidth, int newHeight, const T* newData)
{
	NOT_NULL (newData);

	allocate (newWidth, newHeight);	
	NOT_NULL (data);

	memcpy (data, newData, width * height * sizeof (T));  //lint !e737  //-- sizeof
}

//-------------------------------------------------------------------

template <class T>
const T* Array2d<T>::getData () const
{
	return data;
}

//-------------------------------------------------------------------

template <class T>
const T& Array2d<T>::getData (int x, int z) const
{
	NOT_NULL (data);
	DEBUG_FATAL (isEmpty (), ("Array2d<T> is empty"));
	DEBUG_FATAL (x < 0 || x >= width, ("x is out of range"));
	DEBUG_FATAL (z < 0 || z >= height, ("z is out of range"));

	return data [z * width + x];
}

//-------------------------------------------------------------------

template<class T>
T& Array2d<T>::getData (int x, int z)
{
	NOT_NULL (data);
	DEBUG_FATAL (isEmpty (), ("Array2d<T> is empty"));
	DEBUG_FATAL (x < 0 || x >= width, ("x is out of range"));
	DEBUG_FATAL (z < 0 || z >= height, ("z is out of range"));

	return data [z * width + x];
}

//-------------------------------------------------------------------

template <class T>
void Array2d<T>::setData (int x, int z, const T& newData)
{
	NOT_NULL (data);
	DEBUG_FATAL (isEmpty (), ("Array2d<T> is empty"));
	DEBUG_FATAL (x < 0 || x >= width, ("x is out of range"));
	DEBUG_FATAL (z < 0 || z >= height, ("z is out of range"));

	data [z * width + x] = newData;
}

//-------------------------------------------------------------------

template <class T>
void Array2d<T>::makeZero ()
{
	NOT_NULL (data);
	DEBUG_FATAL (isEmpty (), ("Array2d<T> is empty"));

	memset (data, 0, width * height * sizeof (T));  //lint !e737  //-- sizeof
}

//-------------------------------------------------------------------

template <class T>
void Array2d<T>::makeValue (const T& newValue)
{
	NOT_NULL (data);
	DEBUG_FATAL (isEmpty (), ("Array2d<T> is empty"));

	int i;
	int j;
	for (j = 0; j < height; j++)
		for (i = 0; i < width; i++)
			data [j * width + i] = newValue;
}

//-------------------------------------------------------------------

template <class T>
void Array2d<T>::makeCopy (const Array2d<T>& rhs)
{
	DEBUG_FATAL (width != rhs.getWidth (), ("width (%i) != rhs.getWidth (%i)", width, rhs.getWidth ()));
	DEBUG_FATAL (height != rhs.getHeight (), ("height (%i) != rhs.getHeight (%i)", height, rhs.getHeight ()));
	NOT_NULL (data);
	NOT_NULL (rhs.data);

	memcpy (data, rhs.data, width * height * sizeof (T));  //lint !e737  //-- sizeof
}

//===================================================================

#endif
