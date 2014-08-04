#include <microscopes/common/relation/dataview.hpp>
#include <microscopes/common/macros.hpp>
#include <microscopes/common/random_fwd.hpp>
#include <microscopes/common/util.hpp>

#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/SparseCore>

#include <random>
#include <iostream>
#include <vector>
#include <set>
#include <memory>

using namespace std;
using namespace Eigen;
using namespace microscopes::common;
using namespace microscopes::common::relation;

static void
CheckDataview2DArray(
    const bool *data,
    const bool *mask,
    size_t n,
    size_t m,
    const dataview &d)
{
  MICROSCOPES_CHECK(d.shape().size() == 2, "not a 2D array");

  // check each slice (row wise)
  for (size_t i = 0; i < n; i++) {
    vector<bool> seen(m, false);
    for (auto &p : d.slice(0, i)) {
      MICROSCOPES_CHECK(p.first[0] == i, "not a valid slice");
      MICROSCOPES_CHECK(p.first[1] < m, "out of bounds");
      const size_t idx = i*m + p.first[1];
      const bool value = p.second.get<bool>(0);
      MICROSCOPES_CHECK(data[idx] == value, "values don't match");
      MICROSCOPES_CHECK(!mask[idx], "data is masked");
      seen[p.first[1]] = true;
    }
    for (size_t j = 0; j < seen.size(); j++) {
      if (seen[j])
        continue;
      const size_t idx = i*m + j;
      MICROSCOPES_CHECK(mask[idx], "data is not masked");
    }
  }

  // check each slice (col wise)
  for (size_t j = 0; j < m; j++) {
    vector<bool> seen(n, false);
    for (auto &p : d.slice(1, j)) {
      MICROSCOPES_CHECK(p.first[0] < n, "OOB");
      MICROSCOPES_CHECK(p.first[1] == j, "invalid slice");
      const size_t idx = p.first[0]*m + j;
      const bool value = p.second.get<bool>(0);
      MICROSCOPES_CHECK(data[idx] == value, "values don't match");
      MICROSCOPES_CHECK(!mask[idx], "data is masked");
      seen[p.first[0]] = true;
    }
    for (size_t k = 0; k < seen.size(); k++) {
      if (seen[k])
        continue;
      const size_t idx = k*m + j;
      MICROSCOPES_CHECK(mask[idx], "data is not masked");
    }
  }
}

static void
test1()
{
  random_device rd;
  rng_t r(rd());
  size_t A = 3;
  size_t B = 4;
  unique_ptr<bool []> data(new bool[A*B]);
  unique_ptr<bool []> masks(new bool[A*B]);

  for (size_t u = 0; u < A; u++) {
    for (size_t m = 0; m < B; m++) {
      const size_t idx = u*B + m;
      // coin flip to see if this data is present
      if (bernoulli_distribution(0.2)(r)) {
        masks[idx] = false;
        // coin flip to see if friends
        data[idx] = bernoulli_distribution(0.8)(r);
      } else {
        masks[idx] = true;
      }
    }
  }

  unique_ptr<dataview> view(
    new row_major_dense_dataview(
        reinterpret_cast<const uint8_t *>(data.get()), masks.get(),
        {A, B}, runtime_type(TYPE_B)));
  CheckDataview2DArray(data.get(), masks.get(), A, B, *view);

  unique_ptr<bool []> data1(new bool[A*B]);
  unique_ptr<bool []> masks1(new bool[A*B]);
  for (size_t u = 0; u < A; u++) {
    for (size_t m = 0; m < B; m++) {
      const size_t idx = u*B + m;
      if (!idx) {
        masks1[idx] = false;
        data1[idx] = true;
      } else {
        masks1[idx] = true;
      }
    }
  }
  unique_ptr<dataview> view1(
    new row_major_dense_dataview(
        reinterpret_cast<const uint8_t *>(data1.get()), masks1.get(),
        {A, B}, runtime_type(TYPE_B)));
  CheckDataview2DArray(data1.get(), masks1.get(), A, B, *view1);

  cout << "test1 completed" << endl;
}

static void
Check2D_I32RelationsEqual(const ArrayXXi &truth,
                          const dataview &d, // assume int32 values for now
                          bool zeros_should_be_absent)
{
  MICROSCOPES_CHECK(d.shape().size() == 2, "not a matrix");
  MICROSCOPES_CHECK((size_t)truth.rows() == d.shape()[0], "rows mismatch");
  MICROSCOPES_CHECK((size_t)truth.cols() == d.shape()[1], "cols mismatch");
  MICROSCOPES_CHECK(d.type() == runtime_type(TYPE_I32), "wrong type");

  {
    const size_t expected = zeros_should_be_absent ?
        size_t((truth != 0).count()) :
        (d.shape()[0] * d.shape()[1]);
    set<vector<size_t>> seen;
    size_t count = 0;
    for (size_t i = 0; i < d.shape()[0]; i++) {
      for (const auto &p : d.slice(0, i)) {
        const auto it = seen.insert(p.first);
        MICROSCOPES_CHECK(it.second, "duplicate indices");
        MICROSCOPES_CHECK(p.first.size() == 2, "bad indices");
        MICROSCOPES_CHECK(
            truth(p.first[0], p.first[1]) == p.second.get<int32_t>(),
            "value mismatch");
        count++;
        //cout << "idx: " << p.first << ", value: " << p.second.get<int32_t>() << endl;
      }
    }
    MICROSCOPES_CHECK(expected == count, "wrong number seen");
  }

  { // row slices
    const auto nnzs = (truth != 0).rowwise().count();
    for (size_t i = 0; i < d.shape()[0]; i++) {
      const size_t expected = zeros_should_be_absent ?
          size_t(nnzs(i)) : d.shape()[1];
      set<vector<size_t>> seen;
      size_t count = 0;
      //cout << "row slice " << i << endl;
      for (const auto &p : d.slice(0, i)) {
        const auto it = seen.insert(p.first);
        MICROSCOPES_CHECK(it.second, "duplicate indices");
        MICROSCOPES_CHECK(p.first.size() == 2, "bad indices");
        MICROSCOPES_CHECK(
            truth(p.first[0], p.first[1]) == p.second.get<int32_t>(),
            "value mismatch");
        count++;
        //cout << "  idx: " << p.first << ", value: " << p.second.get<int32_t>() << endl;
      }
      MICROSCOPES_CHECK(expected == count, "wrong number seen (row slice)");
    }
  }

  { // col slices
    const auto nnzs = (truth != 0).colwise().count();
    for (size_t j = 0; j < d.shape()[1]; j++) {
      const size_t expected = zeros_should_be_absent ?
          size_t(nnzs(j)) : d.shape()[0];
      set<vector<size_t>> seen;
      size_t count = 0;
      //cout << "col slice " << j << endl;
      for (const auto &p : d.slice(1, j)) {
        const auto it = seen.insert(p.first);
        MICROSCOPES_CHECK(it.second, "duplicate indices");
        MICROSCOPES_CHECK(p.first.size() == 2, "bad indices");
        MICROSCOPES_CHECK(
            truth(p.first[0], p.first[1]) == p.second.get<int32_t>(),
            "value mismatch");
        count++;
        //cout << "  idx: " << p.first << ", value: " << p.second.get<int32_t>() << endl;
      }
      MICROSCOPES_CHECK(expected == count, "wrong number seen (col slice)");
    }
  }
}

typedef SparseMatrix<int32_t, RowMajor, int32_t> CSRI32Matrix;
typedef SparseMatrix<int32_t, ColMajor, int32_t> CSCI32Matrix;

template <int Order>
static void
FillSparseMatrix(SparseMatrix<int32_t, Order, int32_t> &sparse,
                 ArrayXXi &dense)
{
  const size_t rows = dense.rows();
  const size_t cols = dense.cols();
  typedef Triplet<int32_t> T;
  vector<T> data;

  size_t nnz = 0;
  for (size_t i = 0; i < rows; i++)
    for (size_t j = 0; j < cols; j++) {
      if (dense(i, j)) {
        data.emplace_back(i, j, dense(i, j));
        nnz++;
      }
    }

  sparse = SparseMatrix<int32_t, Order, int32_t>(rows, cols);
  sparse.setFromTriplets(data.begin(), data.end());
  sparse.finalize(); // what is this?
  sparse.makeCompressed(); // makes same as scipy repr
  MICROSCOPES_CHECK(sparse.isCompressed(), "compress fail");
  MICROSCOPES_CHECK(nnz == (size_t)sparse.nonZeros(), "nnz wrong");
}

template <typename T>
static inline string
array_to_string(const T *px, size_t n)
{
  vector<T> elems(px, px + n);
  ostringstream oss;
  oss << elems;
  return oss.str();
}

static void
test2()
{
  random_device rd;
  rng_t r(rd());
  size_t A = 10;
  size_t B = 7;
  unique_ptr<int32_t []> data(new int32_t[A*B]);

  ArrayXXi truth(A, B);

  for (size_t u = 0; u < A; u++) {
    for (size_t m = 0; m < B; m++) {
      const size_t idx = u*B + m;
      const uint32_t val = uniform_int_distribution<int32_t>(-2, 2)(r);
      data[idx] = val;
      truth(u, m) = val;
    }
  }

  //cout << "truth: " << truth << endl;

  unique_ptr<dataview> view(
    new row_major_dense_dataview(
        reinterpret_cast<const uint8_t *>(data.get()), nullptr,
        {A, B}, runtime_type(TYPE_I32)));

  Check2D_I32RelationsEqual(truth, *view, false);

  CSRI32Matrix csr;
  CSCI32Matrix csc;
  FillSparseMatrix(csr, truth);
  FillSparseMatrix(csc, truth);

  //cout << "values:" << endl;
  //cout << "  " << array_to_string(csr.valuePtr(), csr.nonZeros()) << endl;
  //cout << "indices:" << endl;
  //cout << "  " << array_to_string(csr.innerIndexPtr(), csr.nonZeros()) << endl;
  //cout << "indptr:" << endl;
  //cout << "  " << array_to_string(csr.outerIndexPtr(), csr.rows() + 1) << endl;

  unique_ptr<dataview> sparseview(
    new compressed_2darray(reinterpret_cast<const uint8_t *>(csr.valuePtr()),
                           reinterpret_cast<const uint32_t *>(csr.innerIndexPtr()),
                           reinterpret_cast<const uint32_t *>(csr.outerIndexPtr()),
                           reinterpret_cast<const uint8_t *>(csc.valuePtr()),
                           reinterpret_cast<const uint32_t *>(csc.innerIndexPtr()),
                           reinterpret_cast<const uint32_t *>(csc.outerIndexPtr()),
                           A,
                           B,
                           runtime_type(TYPE_I32)));

  Check2D_I32RelationsEqual(truth, *sparseview, true);

  cout << "test2 completed" << endl;
}

int
main(void)
{
  test1();
  test2();
  return 0;
}
