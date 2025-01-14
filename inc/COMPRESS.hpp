#pragma once

#include "HUFFMAN.hpp"
#include "SOCA.hpp"
#include "UTIL.hpp"
#include "fmt/base.h"

#include <span>

namespace impl::compress {

template<size_t rule, typename ItrFreq, typename ItrData>
uint8_t section(ItrFreq freqBegin,
                ItrFreq freqEnd,
                ItrData dataBegin,
                ItrData dataEnd) {
  ++(*freqBegin);
  double best_entropy = util::calculate_entropy(freqBegin, freqEnd);
  --(*freqBegin);

  uint8_t best_count = 0;

  ++(*(freqBegin + 1));

  for (uint8_t count = 0; count < 32; ++count) {
    for (auto itr = dataBegin; itr != dataEnd; ++itr) {
      --(*(freqBegin + *itr));
    }
    --(*(freqBegin + count + 1));

    if (count % 2 == 0) {
      soca::forward_front<rule>(dataBegin, dataEnd);
    } else {
      soca::forward_back<rule>(dataBegin, dataEnd);
    }

    for (auto itr = dataBegin; itr != dataEnd; ++itr) {
      ++(*(freqBegin + *itr));
    }
    ++(*(freqBegin + count + 1));

    const double entropy = util::calculate_entropy(freqBegin, freqEnd);
    if (entropy < best_entropy) {
      best_entropy = entropy;
      best_count   = count + 1;
    }
  }

  for (auto itr = dataBegin; itr != dataEnd; ++itr) {
    --(*(freqBegin + *itr));
  }
  --(*(freqBegin + 32));

  for (uint8_t count = 32; count > best_count; --count) {
    if (count % 2 == 0) {
      soca::reverse_back<rule>(dataBegin, dataEnd);
    } else {
      soca::reverse_front<rule>(dataBegin, dataEnd);
    }
  }

  for (auto itr = dataBegin; itr != dataEnd; ++itr) {
    ++(*(freqBegin + *itr));
  }
  ++(*(freqBegin + best_count));

  return best_count;
}

template<size_t rule, typename Itr>
void section_decompress(Itr begin, Itr end, uint8_t count) {
  for (uint8_t i = count; i > 0; --i) {
    if (count % 2 == 0) {
      soca::reverse_back<rule>(begin, end);
    } else {
      soca::reverse_front<rule>(begin, end);
    }
  }
}

template<size_t period, typename ItrBase, typename ItrWeave>
requires std::random_access_iterator<ItrBase> &&
         std::random_access_iterator<ItrWeave>
class weaving_iterator {
public:
  using iterator_category = std::forward_iterator_tag;
  using value_type        = uint8_t;
  using difference_type   = ptrdiff_t;
  using pointer           = value_type*;
  using reference         = value_type&;

private:
  ItrBase         base;
  ItrWeave        weave;
  difference_type count;

public:
  constexpr weaving_iterator(): base(), weave(), count(0) {
  }

  constexpr weaving_iterator(ItrBase base, ItrWeave weave, ptrdiff_t count = 0):
    base(base),
    weave(weave),
    count(count) {
  }

  constexpr std::strong_ordering operator<=>(
  const weaving_iterator& other) const {
    return count <=> other.count;
  }

  constexpr bool operator==(const weaving_iterator& other) const {
    return count == other.count;
  }

  constexpr bool operator!=(const weaving_iterator& other) const {
    return !(*this == other);
  }

  constexpr value_type operator*() const {
    return count % period == 0 ? *weave : *base;
  }

  constexpr weaving_iterator& operator++() {
    if (count % period == 0) {
      ++weave;
    } else {
      ++base;
    }
    ++count;
    return *this;
  }

  constexpr weaving_iterator operator++(int) {
    weaving_iterator copy = *this;
    ++(*this);
    return copy;
  }
};

template<size_t period, typename ItrBase, typename ItrWeave>
constexpr weaving_iterator<period, ItrBase, ItrWeave>
weaving_begin(ItrBase base_begin, ItrBase base_end, ItrWeave weave_begin) {
  return weaving_iterator<period, ItrBase, ItrWeave>(base_begin,
                                                     weave_begin,
                                                     0);
}

template<size_t period, typename ItrBase, typename ItrWeave>
constexpr weaving_iterator<period, ItrBase, ItrWeave>
weaving_end(ItrBase base_begin, ItrBase base_end, ItrWeave weave_begin) {
  const auto base_size = std::distance(base_begin, base_end);

  return weaving_iterator<period, ItrBase, ItrWeave>(
  base_end,
  weave_begin + (base_size / period + 1),
  base_size + base_size / period + 1);
}

template<size_t period, typename ItrBase, typename ItrWeave>
class deweaving_iterator {
public:
  using iterator_category = std::output_iterator_tag;
  using value_type        = uint8_t;
  using difference_type   = ptrdiff_t;
  using pointer           = value_type*;
  using reference         = value_type&;

private:
  ItrBase   base;
  ItrWeave  weave;
  difference_type count {0};

public:
  constexpr deweaving_iterator(ItrBase base, ItrWeave weave):
    base(base),
    weave(weave) {
  }

  constexpr bool operator==(const deweaving_iterator& other) const {
    return count == other.count;
  }

  constexpr bool operator!=(const deweaving_iterator& other) const {
    return !(*this == other);
  }

  constexpr deweaving_iterator& operator*() {
    return *this;
  }

  constexpr deweaving_iterator& operator++() {
    return *this;
  }

  constexpr deweaving_iterator& operator++(int) {
    return *this;
  }

  constexpr deweaving_iterator& operator=(value_type value) {
    if (count % period == 0) {
      *weave++ = value;
    } else {
      *base++ = value;
    }
    ++count;
    return *this;
  }
};

template<size_t period, typename ItrBase, typename ItrWeave>
constexpr deweaving_iterator<period, ItrBase, ItrWeave> deweaving_begin(
ItrBase  base,
ItrWeave weave) {
  return deweaving_iterator<period, ItrBase, ItrWeave>(base, weave);
}

}    // namespace impl::compress

template<uint8_t rule, size_t section_size, typename ItrIn, typename ItrOut>
void compress(ItrIn begin, ItrIn end, ItrOut out) {
  auto size = std::distance(begin, end);

  std::array<size_t, 256> freq {};
  for (auto itr = begin; itr != end; ++itr) {
    ++freq[*itr];
  }

  std::vector<uint8_t> soca_counts;

  for (ptrdiff_t start = 0; start < size; start += section_size) {
    soca_counts.emplace_back(
    impl::compress::section<rule>(freq.begin(),
                                  freq.end(),
                                  begin + start,
                                  begin + start +
                                  std::min(static_cast<ptrdiff_t>(section_size),
                                           std::distance(begin + start, end))));
  }

  huffman::encode(
  impl::compress::weaving_begin<section_size>(begin, end, soca_counts.begin()),
  impl::compress::weaving_end<section_size>(begin, end, soca_counts.begin()),
  out);

  // todo: freq known here, optimize huffman?
  // todo: data: 1 byte best soca, section_size bytes, ...
}

template<uint8_t rule, size_t section_size, typename ItrIn, typename ItrOut>
void decompress(ItrIn begin, ItrIn end, ItrOut out) {
  std::vector<uint8_t> soca_counts;
  std::vector<uint8_t> sections;

  huffman::decode(begin,
                  end,
                  impl::compress::deweaving_begin<section_size>(
                  std::back_inserter(sections),
                  std::back_inserter(soca_counts)));

  ptrdiff_t section = 0;
  for (size_t start = 0; start < sections.size(); start += section_size) {
    impl::compress::section_decompress<rule>(
    sections.begin() + start,
    sections.begin() + start +
    std::min(static_cast<ptrdiff_t>(section_size),
             std::distance(sections.begin() + start, sections.end())),
    soca_counts[section]);
    ++section;
  }

  std::copy(sections.begin(), sections.end(), out);
}
