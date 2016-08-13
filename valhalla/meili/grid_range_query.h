// -*- mode: c++ -*-
#ifndef MMP_GRID_RANGE_QUERY_H_
#define MMP_GRID_RANGE_QUERY_H_

#include <algorithm>
#include <tuple>
#include <utility>
#include <vector>
#include <unordered_set>
#include <cmath>
#include <stdexcept>

#include <valhalla/midgard/aabb2.h>
#include <valhalla/midgard/linesegment2.h>

#include <valhalla/meili/grid_traversal.h>


namespace valhalla{
namespace meili {


template <typename item_t, typename coord_t>
class GridRangeQuery
{
 public:
  GridRangeQuery(const midgard::AABB2<coord_t>& bbox, float square_width, float square_height):
      bbox_(bbox),
      square_width_(square_width),
      square_height_(square_height),
      ncols_((bbox.maxx() - bbox.minx()) / square_width),
      nrows_((bbox.maxy() - bbox.miny()) / square_height),
      grid_(bbox.minx(), bbox.miny(), square_width, square_height, ncols_, nrows_),
      items_()
  { items_.resize(ncols_ * nrows_); }

  const midgard::AABB2<coord_t>& bbox() const
  { return bbox_; }

  int nrows() const
  { return nrows_; }

  int ncols() const
  { return ncols_; }

  float square_width() const
  { return square_width_; }

  float square_height() const
  { return square_height_; }

  const std::vector<item_t>&
  GetItemsInSquare(int col, int row) const
  {
    if (!(0 <= col && col < ncols_ && 0 <= row && row < nrows_)) {
      throw std::runtime_error("SQUARE(" + std::to_string(col)  + " " + std::to_string(row)
                               + ") is out of the grid bounds (" + std::to_string(ncols_) + "x"
                               + std::to_string(nrows_) + " squares)");
    }
    return items_[col + row * ncols_];
  }

  // Index a line segment into the grid
  void AddLineSegment(const item_t& item, const midgard::LineSegment2<coord_t>& segment)
  {
    for (const auto& square: grid_.Traverse(segment)) {
      ItemsInSquare(square.first, square.second).push_back(item);
    }
  }

  // Query all items that intersects with the range
  std::unordered_set<item_t>
  Query(const midgard::AABB2<coord_t>& range) const
  {
    int mincol, minrow, maxcol, maxrow;
    std::tie(mincol, minrow) = grid_.SquareAtPoint(range.minpt());
    std::tie(maxcol, maxrow) = grid_.SquareAtPoint(range.maxpt());

    // Normalize
    mincol = std::max(0, std::min(mincol, ncols_ - 1));
    maxcol = std::max(0, std::min(maxcol, ncols_ - 1));
    minrow = std::max(0, std::min(minrow, nrows_ - 1));
    maxrow = std::max(0, std::min(maxrow, nrows_ - 1));

    std::unordered_set<item_t> items;

    for (int row = minrow; row <= maxrow; ++row) {
      for (int col = mincol; col <= maxcol; ++col) {
        const auto& squared_items = GetItemsInSquare(col, row);
        items.insert(squared_items.begin(), squared_items.end());
      }
    }

    return items;
  }

 private:
  std::vector<item_t>& ItemsInSquare(int col, int row)
  {
    if (!(0 <= col && col < ncols_ && 0 <= row && row < nrows_)) {
      throw std::runtime_error("SQUARE(" + std::to_string(col)  + " " + std::to_string(row)
                               + ") is out of the grid bounds (" + std::to_string(ncols_) + "x"
                               + std::to_string(nrows_) + " squares)");
    }
    return items_[col + row * ncols_];
  }

  midgard::AABB2<coord_t> bbox_;
  float square_width_, square_height_;
  int ncols_, nrows_;
  GridTraversal<coord_t> grid_;
  std::vector<std::vector<item_t> > items_;
};

}
}
#endif // MMP_GRID_RANGE_QUERY_H_
