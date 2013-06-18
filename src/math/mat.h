#ifndef __MAT_H__
#define __MAT_H__

#include <iostream>
#include <vector>

namespace ltp {
namespace math {

using namespace std;

/*
 * A class for vector
 */
template <typename T>
class Vec {
private:
    int nn;
    T * v;
public:
    Vec() : nn(0), v(0) {}

    ~Vec() {
        dealloc();
    }

    // zero-based array
    explicit Vec(const int n) : nn(0), v(0) {
        resize(n);
    }

    // initialize to constant value
    Vec(const T &a, const int n) : nn(n), v(new T[n]) {
        for (int i = 0; i < n; ++ i) {
            v[i] = a;
        }
    }

    // initialize to array
    Vec(const T *a, const int n) : nn(n), v(new T[n]) {
        for (int i = 0; i < n; ++ i) {
            v[i] = *a;
            a ++;
        }
    }

    // copy constructor
    Vec(const Vec<T> &rhs): nn(rhs.nn), v(new T[nn]) {
        for (int i = 0; i < nn; ++ i) {
            v[i] = rhs[i];
        }
    }

    Vec & resize(const int n) {
        if (nn != n) {
            if (v != 0) {
                delete [] (v);
            }
            nn = n;
            v = new T[n];
        }
        return *this;
    }

    Vec & operator=(const Vec &rhs) {
        if (this != &rhs) {
            if (nn != rhs.nn) {
                if (v != 0) {
                    delete [] (v);
                }
                nn = rhs.nn;
                v = new T[nn];
            }

            for (int i = 0; i < nn; ++ i) v[i] = rhs[i];
        }
        return *this;
    }

    Vec & operator=(const T& a) {
        for (int i = 0; i < nn; ++ i) {
            v[i] = a;
        }
        return *this;
    }

    Vec & operator=(const std::vector<T> &a) {
        if (nn != a.size()) {
            if (v != 0) {
                delete [] (v);
            }
            nn = a.size();
            v = new T[nn];
        }

        for (int i = 0; i < nn; ++ i) v[i] = a[i];
        return *this;
    }

    inline T & operator [](const int i) {
        return v[i];
    }

    inline const T & operator [](const int i) const {
        return v[i];
    }

    inline int size() const {
        return nn;
    }

    inline void dealloc() {
        if (v != 0) {
            delete [] (v);
            v = 0;
            nn = 0;
        }
    }

    inline T * c_buf() {
        return v;
    }
};  //  end for class Vec

/*
 * A class for matrix(2d)
 */
template <typename T>
class Mat {
private:
    int nn;
    int mm;
    int tot_sz;
    T ** v;
public:
    ~Mat() {
        dealloc();
    }

    void dealloc() {
        if (v != 0) {
            delete [] (v[0]);
            delete [] (v);
            v = 0;
            nn = 0;
            mm = 0;
            tot_sz = 0;
        }
    }

    T * c_buf() {
        if (v) {
            return v[0];
        } else {
            return 0;
        }
    }

    Mat() : nn(0), mm(0), tot_sz(0), v(0) {}

    Mat & resize(const int n, const int m) {
        if (nn != n || mm != m) {
            dealloc();
            nn = n;
            mm = m;
            tot_sz = n * m;

            v = new T*[n];
            v[0] = new T[tot_sz];

            for (int i = 1; i < n; ++ i) {
                v[i] = v[i - 1] + m;
            }
        }
        return *this;
    }

    Mat(const int n, const int m) 
        : nn(0), mm(0), tot_sz(0), v(0) {
        resize(n, m);
    }

    Mat(const T & a, const int n, const int m) 
        : nn(0), mm(0), tot_sz(0), v(0) {
        resize(n, m);
        for (int i = 0; i < n; ++ i) {
            for (int j = 0; j < m; ++ j) {
                v[i][j] =a;
            }
        }
    }

    Mat(const T * a, const int n, const int m) {
        resize(n, m);
        for (int i = 0; i < n; ++ i) {
            for (int j = 0; j < m; ++ j) {
                v[i][j] = *a;
                ++ a;
            }
        }
    }

    Mat(const Mat & rhs) {
        resize(rhs.nn, rhs.mm);
        for (int i = 0; i < nn; ++ i) {
            for (int j = 0; j < mm; ++ j) {
                v[i][j] = rhs[i][j];
            }
        }
    }

    Mat & operator= (const Mat &rhs) {
        if (this != &rhs) {
            resize(rhs.nn, rhs.mm);

            for (int i = 0; i < nn; ++ i) {
                for (int j = 0; j < mm; ++ j) {
                    v[i][j] = rhs[i][j];
                }
            }
        }
        return *this;
    }

    Mat & operator= (const T & a) {
        for (int i = 0; i < nn; ++ i) {
            for (int j = 0; j < mm; ++ j) {
                v[i][j] = a;
            }
        }
        return *this;
    }

    inline T * operator[] (const int i) {
        return v[i];
    }

    inline const T * operator[] (const int i) const {
        return v[i];
    }

    inline int nrows() const {
        return nn;
    }

    inline int ncols() const {
        return mm;
    }

    inline int total_size() const {
        return tot_sz;
    }
};  //  end for class Mat

/*
 *
 *
 */
template <typename T>
class Mat3 {
private:
    int nn;
    int mm;
    int kk;
    int tot_sz;

    T ***v;
public:
    Mat3() : nn(0), mm(0), kk(0), tot_sz(0), v(0) {}
    ~Mat3() {
        dealloc();
    }

    void dealloc() {
        if (v != 0) {
            delete [] (v[0][0]);
            delete [] (v[0]);
            delete [] (v);
            v = 0;
            nn = 0;
            mm = 0;
            kk = 0;
            tot_sz = 0;
        }
    }

    T * c_buf() {
        if (v) {
            return v[0][0];
        } else {
            return NULL;
        }
    }

    Mat3(const int n, const int m, const int k) 
        : nn(0), mm(0), kk(0), tot_sz(0), v(0) {
        resize(n, m, k);
    }

    Mat3 & resize(const int n, const int m, const int k) {
        if (nn != n || mm != m || kk != k) {
            dealloc();

            nn = n;
            mm = m;
            kk = k;
            tot_sz = n * m * k;

            v = new T**[n];
            v[0] = new T*[n * m];
            v[0][0] = new T[tot_sz];

            int i, j;
            for (j = 1; j < m; ++ j) {
                v[0][j] = v[0][j - 1] + k;
            }

            for (i = 1; i < n; ++ i) {
                v[i] = v[i - 1] + m;
                v[i][0] = v[i - 1][0] + m * k;

                for (j = 1; j < m; ++ j) {
                    v[i][j] = v[i][j - 1] + k;
                }
            }
        }

        return *this;
    }

    Mat3 & operator= (const T &a) {
        for (int i = 0; i < nn; ++ i) {
            for (int j = 0; j < mm; ++ j) {
                for (int k = 0; k < kk; ++ k) {
                    v[i][j][k] = a;
                }
            }
        }

        return *this;
    }

    inline T** operator[] (const int i) {
        return v[i];
    }

    inline const T * const * operator[] (const int i) const {
        return v[i];
    }

    inline int dim1() const {
        return nn;
    }

    inline int dim2() const {
        return mm;
    }

    inline int dim3() const {
        return kk;
    }

};  //  end for class Mat3

}   //  end for namespace math
}   //  end for namespace ltp

#endif  // end for __MAT_H__
