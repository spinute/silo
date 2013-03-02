#ifndef _NDB_VARKEY_H_
#define _NDB_VARKEY_H_

#include <endian.h>
#include <stdint.h>
#include <string.h>

#include <iostream>
#include <string>

#include "imstring.h"
#include "macros.h"
#include "util.h"
#include "xbuf.h"

class varkey {
  friend std::ostream &operator<<(std::ostream &o, const varkey &k);
public:
  inline varkey() : p(NULL), l(0) {}

  inline varkey(const uint8_t *p, size_t l)
    : p(p), l(l)
  {
  }

  explicit inline varkey(const std::string &s)
    : p((const uint8_t *) s.data()), l(s.size())
  {
  }

  explicit inline varkey(const xbuf &s)
    : p((const uint8_t *) s.data()), l(s.size())
  {
  }

  explicit inline varkey(const char *s)
    : p((const uint8_t *) s), l(strlen(s))
  {
  }

  explicit inline varkey(const imstring &s)
    : p(s.data()), l(s.size())
  {
  }

  inline bool
  operator==(const varkey &that) const
  {
    if (size() != that.size())
      return false;
    return memcmp(data(), that.data(), size()) == 0;
  }

  inline bool
  operator!=(const varkey &that) const
  {
    return !operator==(that);
  }

  inline bool
  operator<(const varkey &that) const
  {
    int r = memcmp(data(), that.data(), std::min(size(), that.size()));
    return r < 0 || (r == 0 && size() < that.size());
  }

  inline bool
  operator>=(const varkey &that) const
  {
    return !operator<(that);
  }

  inline bool
  operator<=(const varkey &that) const
  {
    int r = memcmp(data(), that.data(), std::min(size(), that.size()));
    return r < 0 || (r == 0 && size() <= that.size());
  }

  inline bool
  operator>(const varkey &that) const
  {
    return !operator<=(that);
  }

  inline uint64_t
  slice() const
  {
    uint64_t ret = 0;
    uint8_t *rp = (uint8_t *) &ret;
    for (size_t i = 0; i < std::min(l, size_t(8)); i++)
      rp[i] = p[i];
    return util::host_endian_trfm<uint64_t>()(ret);
  }

  inline varkey
  shift() const
  {
    INVARIANT(l >= 8);
    return varkey(p + 8, l - 8);
  }

  inline varkey
  shift_many(size_t n) const
  {
    INVARIANT(l >= 8 * n);
    return varkey(p + 8 * n, l - 8 * n);
  }

  inline size_t
  size() const
  {
    return l;
  }

  inline const uint8_t *
  data() const
  {
    return p;
  }

  inline
  std::string str() const
  {
    return std::string((const char *) p, l);
  }

private:
  const uint8_t *p;
  size_t l;
};

inline std::ostream &
operator<<(std::ostream &o, const varkey &k)
{
  o << util::hexify(k.str());
  return o;
}

template <typename T, typename Trfm = util::big_endian_trfm<T> >
class obj_varkey : public varkey {
public:
  inline obj_varkey() : varkey() {}

  inline obj_varkey(T t)
    : varkey((const uint8_t *) &obj, sizeof(T)), obj(Trfm()(t))
  {
  }

private:
  T obj;
};

typedef obj_varkey<uint8_t>  u8_varkey;
typedef obj_varkey<int8_t>   s8_varkey;
typedef obj_varkey<uint16_t> u16_varkey;
typedef obj_varkey<int16_t>  s16_varkey;
typedef obj_varkey<uint32_t> u32_varkey;
typedef obj_varkey<int32_t>  s32_varkey;
typedef obj_varkey<uint64_t> u64_varkey;
typedef obj_varkey<int64_t>  s64_varkey;

#endif /* _NDB_VARKEY_H_ */
