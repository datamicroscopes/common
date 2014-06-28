#include <microscopes/common/dataview.hpp>
#include <microscopes/common/util.hpp>

#include <cassert>
#include <sstream>
#include <iostream>

using namespace std;
using namespace microscopes::common;

static vector<string>
runtime_type_strings(const vector<runtime_type_info> &types)
{
  vector<string> ret;
  ret.reserve(types.size());
  for (auto t : types)
    ret.push_back(runtime_type_traits::TypeInfoString(t));
  return ret;
}

string
row_accessor::debug_str() const
{
  vector<string> values_repr;
  values_repr.reserve(types_->size());
  for (size_t i = 0; i < types_->size(); i++) {
    if (!mask_ || !mask_[i])
      values_repr.push_back(
          runtime_type_traits::ToString((*types_)[i], data_ + (*offsets_)[i]));
    else
      values_repr.push_back("--");
  }
  ostringstream oss;
  oss << "{"
      << "types=" << runtime_type_strings(*types_) << ", "
      << "values="<< values_repr << ", ";
  if (mask_) {
    vector<string> mask_repr;
    mask_repr.reserve(types_->size());
    for (size_t i = 0; i < types_->size(); i++)
      mask_repr.push_back(mask_[i] ? "true" : "false");
    oss << "mask=" << mask_repr;
  } else
    oss << "mask=null";
  oss << "}";
  return oss.str();
}

string
row_mutator::debug_str() const
{
  ostringstream oss;
  row_accessor acc(data_, nullptr, types_, offsets_);
  oss << "{view=" << acc.debug_str() << "}";
  return oss.str();
}

dataview::dataview(size_t n, const vector<runtime_type_info> &types)
  : n_(n), types_(types), rowsize_()
{
  const auto ret = runtime_type_traits::GetOffsetsAndSize(types);
  offsets_ = ret.first;
  rowsize_ = ret.second;
}

row_major_dataview::row_major_dataview(
    const uint8_t *data,
    const bool *mask,
    size_t n,
    const vector<runtime_type_info> &types)
    : dataview(n, types), data_(data), mask_(mask), pos_()
{
    //cout << "types:" << endl;
    //for (auto t : types)
    //    cout << "  " << t << endl;
    //cout << "offsets:" << endl;
    //for (auto off : offsets())
    //    cout << "  " << off << endl;
    //cout << "rowsize:" << rowsize() << endl;
}

row_accessor
row_major_dataview::get() const
{
  const size_t actual_pos = pi_.empty() ? pos_ : pi_[pos_];
  const uint8_t *cursor = data_ + rowsize() * actual_pos;
  const bool *mask_cursor = !mask_ ? nullptr : mask_ + types().size() * actual_pos;
  return row_accessor(cursor, mask_cursor, &types(), &offsets());
}

size_t
row_major_dataview::index() const
{
  const size_t actual_pos = pi_.empty() ? pos_ : pi_[pos_];
  return actual_pos;
}

void
row_major_dataview::next()
{
  assert(!end());
  pos_++;
}

void
row_major_dataview::reset()
{
  pos_ = 0;
}

bool
row_major_dataview::end() const
{
  return pos_ == size();
}

void
row_major_dataview::permute(rng_t &rng)
{
  util::permute(pi_, size(), rng);
}
