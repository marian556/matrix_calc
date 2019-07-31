/*
 * matrix.h
 *
 *  Created on: Jun 15, 2019
 *      Author: marian
 */

#ifndef MATRIX_H_
#define MATRIX_H_

#include <vector>
#include <iostream>

template<typename Elem>
struct MatrixView;

struct matrix_exception:std::exception
{
	explicit matrix_exception(const std::string& msg):msg(msg){}

	virtual const char*
	    what() const noexcept
	{
		return msg.c_str();
	}
	std::string msg;
};



template <typename elemT>
struct Matrix
{
	Matrix() {}

	Matrix(Matrix&& other):data(std::move(other.data)){}

	Matrix(const Matrix& other):data(other.data){}

	Matrix(std::initializer_list< std::vector<elemT> > l) : data(l){}

	Matrix(std::vector<std::vector<elemT> >&& other)
		:data(std::move(other)){};

	Matrix(const std::vector<std::vector<elemT> >& other)//converting constructor
		:data(std::move(other)){};

	void operator =(Matrix&& other)
	{
		data=std::move(other.data);
	}

	void operator =(const Matrix& other)
	{
			data=other.data;
	}

	void operator =(const elemT& el)
	{
		data.clear();
		data.push_back(std::vector{el});
	}

	void operator =(elemT&& el)
	{
		data.clear();
		data.push_back(std::vector{std::move(el)});
	}


	bool is_square() const
	{
		return (width()==height());
	}

	elemT det() const
	{

		return MatrixView<elemT>(this).det();
	}

	void print(std::ostream& os) const
	{
	 if (empty())
			return;
	 const Matrix& m=*this;
	 os << '[';
	 for(int i=0;i<(int)m.height();i++)
	 {
		 int sz=m[i].size();
		 os << '[';
		 for(int j=0;j<sz;j++)
		 {
			 os << m[i][j] ;
			 if (j!=sz-1)
				 os << ",";
		 }
		 os << ']';
		 if (i!=(int)m.height()-1)
			 os << ",";
	 }
	 os << ']' << std::flush;
	}

	void print() const
	{
		print(std::cout);
	}

	void invert()
	{
		for(auto& row:data)
			for(auto& el:row)
				el=-el;
	}

	bool check_all_rows_same_size() const
	{
		int sz=data.size();
		if (sz<2)
			return true;
		unsigned  width0=data[0].size();
		for(int i=1;i<sz;i++)
		{
			if (data[i].size()!=width0)
				return false;
		}
		return true;
	}

	bool is_addition_compatible(const Matrix& other) const
	{
		if ((!check_all_rows_same_size())||(!other.check_all_rows_same_size()))
			return false;
		if (height()!=other.height())
			return false;
		if (height()==0)
			return true;//?? matrix 0x0
		return (width()==other.width());
	}



	void throw_if_not_addition_compatible(const Matrix& other) const
	{
		if (height()==0)
			throw matrix_exception("Empty matrix on lhs of additon.");

		if (other.height()==0)
			throw matrix_exception("Empty matrix on rhs of additon.");

		if (!check_all_rows_same_size())
			throw matrix_exception("Invalid matrix with rows of different sizes on lhs for addition/substraction.");

		if (!other.check_all_rows_same_size())
			throw matrix_exception("Invalid matrix with rows of different sizes on rhs for addition/substraction.");

		if (height()!=other.height())
			throw matrix_exception("Incompatible matrix heights for addition/substraction.");

		if (height()!=other.height())
			throw matrix_exception("Incompatible matrix heights for addition/substraction.");
	}

	void throw_if_not_multiplication_compatible(const Matrix& other) const
	{
		if (height()==0)
			throw matrix_exception("Empty matrix on lhs of multiplication.");

		if (other.height()==0)
			throw matrix_exception("Empty matrix on rhs of multiplication.");

		if (!check_all_rows_same_size())
			throw matrix_exception("Invalid matrix with rows of different sizes on lhs for multiplication.");

		if (!other.check_all_rows_same_size())
			throw matrix_exception("Invalid matrix with rows of different sizes on rhs for multiplication.");

		if (width()!=other.height())
			throw matrix_exception("Incompatible matrix dimensions for multiplication. Left matrix width != right matrix height.");
	}

	bool is_multiply_compatible(const Matrix& other) const
	{
		if ((!check_all_rows_same_size())||(!other.check_all_rows_same_size()))
			return false;
		if (height()<1)
			return false;
		return (width()==other.height());
	}

	bool multiply(const Matrix& mr)
	//
	{
		auto& ml=data;
		if (mr.data.size()==0)
			return false;
		int new_width=mr.width();
		int new_height=data.size();//ml.height()
		if (new_height==0)
			return false;
		Matrix new_matrix;
		int N=std::min(ml[0].size(),mr.height());
		for (int r=0;r<new_height;r++)
		{
			std::vector<elemT> new_row;
			const std::vector<elemT>& left_row=data[r];
			if ((int)left_row.size()!=N)
				return false;
			for (int c=0;c<new_width;c++)
			{
				elemT sum=0;
				for(int i=0;i<N;i++)
					sum += left_row[i]*mr[i][c];
				new_row.push_back(sum);

			}
			new_matrix.data.push_back(std::move(new_row));
		}
		data=std::move(new_matrix.data);
		return true;
	}

	template <typename Functor>
	inline void for_each(Functor f)
	{
		int r=0;
		for(auto& row:data)
		{
			int c=0;
			for(auto& el:row)
			{
				f(r,c,el);
				c++;
			}
			r++;
		}
	}

	bool empty() const
	{
		return ((data.size()==0) || (width()==0));
	}

	void transpose()//widht and height can be different
	{
		if (!check_all_rows_same_size())
			throw matrix_exception("Invalid matrix with rows of different sizes.");
		Matrix<elemT> newm;
		int h=height();
		int w=width();
		for (int col=0;col<w;col++)
		{
			std::vector<elemT> new_row;
			for (int row=0;row<h;row++)
			{
				new_row.push_back(std::move(data[row][col]));//destructive move
			}
			newm.data.push_back(std::move(new_row));
		}
		std::swap(data,newm.data);
	}

	Matrix& operator += (const Matrix& m2)
	{
		throw_if_not_addition_compatible(m2);
		for_each([&m2](int r,int c,elemT& lhs){ lhs+=m2[r][c]; });
		//add(m2,true);
		return *this;
	}

	Matrix& operator -= (const Matrix& m2)
	{
		throw_if_not_addition_compatible(m2);
		for_each([&m2](int r,int c,elemT& lhs){ lhs-=m2[r][c]; });
		//add(m2,false);
		return *this;
	}

	Matrix& operator *= (const Matrix& m2)
	{
		throw_if_not_multiplication_compatible(m2);
		multiply(m2);
		return *this;
	}


	//Row-major order (lexographical access order)
	elemT& at(int row,int col) {
		std::vector<elemT>& mrow=data.at(row);
		return mrow.at(col);
	}

	//Row-major order (lexographical access order)
	const std::vector<elemT>& operator [] (int row) const
	{
		return data[row];
	}

	std::vector<elemT>& operator [] (int row)
	{
			return data[row];
	}

	size_t height() const { return data.size(); }
	size_t width(int row=0) const { return data[row].size(); }

	std::vector<std::vector<elemT> > data;
	friend MatrixView<elemT>;
};

template <typename T>
std::ostream& operator << (std::ostream& os,const Matrix<T>& m )
{
	m.print(os);
	return os;
};

template <typename T>
inline T det(Matrix<T>& m1)
{
	return m1.det();
}

template <typename T>
inline T is_square(Matrix<T>& m1)
{
	return m1.is_square();
}

template <typename T>
void flip_sign_in_place(Matrix<T>& matrix )
{
	matrix.invert();
}


template<typename Elem>
struct MatrixView
{

	MatrixView(const Matrix<Elem>* matrix,int remove_row0=-1,int remove_col0=-1)
		:matrix(matrix)
	{
		set_full();
		if ((remove_row0>=0)&&(remove_row0<height()))
			remove_row(remove_row0);
		if ((remove_col0>=0)&&(remove_col0<width()))
			remove_col(remove_col0);
	}

	MatrixView(const MatrixView<Elem>& other,int remove_row0=-1,int remove_col0=-1)
		:matrix(other.matrix),rows_map(other.rows_map),cols_map(other.cols_map)
	{
		if ((remove_row0>=0)&&(remove_row0<height()))
			remove_row(remove_row0);
		if ((remove_col0>=0)&&(remove_col0<width()))
			remove_col(remove_col0);
	}


	MatrixView& operator =(const MatrixView<Elem>& other)
	{
		matrix=other.atrix;
		rows_map=other.rows_map;
		cols_map=other.cols_map;
	}

	const Elem& at(int row,int col) const
	{
		return (*matrix)[rows_map[row]][cols_map[col]];
	}
	int width() const {
		return cols_map.size();
	}
	int height() const
	{
		return rows_map.size();
	}
	void set_full()
	{
		unsigned mymax=0;
		int cnt=0;
		for(auto& row:matrix->data)
		{
			if (row.size()>mymax)
			   mymax=row.size();
			cnt++;
		}
		rows_map.clear();
		cols_map.clear();
		for(int i=0;i<cnt;i++)
			rows_map.push_back(i);
		for(unsigned i=0;i<mymax;i++)
				cols_map.push_back(i);
	}
	void remove_row(int row)
	{
		rows_map.erase(rows_map.begin()+row);
	}
	void remove_col(int col)
	{
		cols_map.erase(cols_map.begin()+col);
	}

	MatrixView<Elem> with_removed(int row, int col) const
	{
		return MatrixView<Elem>(*this,row,col);
	}

	Elem det() const
	{
		if (width()==1)
			return at(0,0);

		Elem sum_det=0;
		int sgn=1;
		for (int i=0;i<width();i++)
		{
			sum_det+=sgn*at(0,i)*with_removed(0,i).det();
			sgn=-sgn;
		}

		return sum_det;
	}

	const Matrix<Elem>* matrix;
	std::vector<int> rows_map;
	std::vector<int> cols_map;
};



#endif /* MATRIX_H_ */
