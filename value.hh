#ifndef _VALUE_H_
#define _VALUE_H_

#include <memory>

#include "constant.hh"
#include "dwgrep.hh"

enum class cmp_result
  {
    less,
    equal,
    greater,
    fail,
  };

template <class T>
cmp_result
compare (T const &a, T const &b)
{
  if (a < b)
    return cmp_result::less;
  if (b < a)
    return cmp_result::greater;
  return cmp_result::equal;
}

// We use this to keep track of types of instances of subclasses of
// class value.  value::as uses this to avoid having to dynamic_cast,
// which is needlessly flexible and slow for our purposes.
class value_type
{
  uint8_t m_code;

  static void register_name (uint8_t code, char const *name);

public:
  static value_type alloc (char const *name);

  explicit value_type (uint8_t code)
    : m_code {code}
  {}

  value_type (uint8_t code, char const *name)
    : m_code {code}
  {
    register_name (code, name);
  }

  value_type (value_type const &that)
    : m_code {that.m_code}
  {}

  // Either returns non-null or crashes.
  char const *name () const;

  uint8_t code () const { return m_code; }

  bool
  operator< (value_type that) const
  {
    return m_code < that.m_code;
  }

  bool
  operator== (value_type that) const
  {
    return m_code == that.m_code;
  }

  bool
  operator!= (value_type that) const
  {
    return m_code != that.m_code;
  }
};

// A domain for slot type constants.
extern constant_dom const &slot_type_dom;

class value
{
  value_type const m_type;
  size_t m_pos;

protected:
  value (value_type t, size_t pos)
    : m_type {t}
    , m_pos {pos}
  {}

  value (value const &that) = default;

public:
  value_type get_type () const { return m_type; }
  constant get_type_const () const;

  virtual ~value () {}
  virtual void show (std::ostream &o) const = 0;
  virtual std::unique_ptr <value> clone () const = 0;
  virtual cmp_result cmp (value const &that) const = 0;

  void
  set_pos (size_t pos)
  {
    m_pos = pos;
  }

  size_t
  get_pos () const
  {
    return m_pos;
  }

  template <class T>
  bool
  is () const
  {
    return T::vtype == m_type;
  }

  template <class T>
  static T *
  as (value *val)
  {
    assert (val != nullptr);
    if (! val->is <T> ())
      return nullptr;
    return static_cast <T *> (val);
  }

  template <class T>
  static T const *
  as (value const *val)
  {
    assert (val != nullptr);
    if (! val->is <T> ())
      return nullptr;
    return static_cast <T const *> (val);
  }

  template <class T>
  static constant
  get_type_const_of ()
  {
    return {T::vtype.code (), &slot_type_dom};
  }
};

std::ostream &operator<< (std::ostream &o, value const &v);

class value_cst
  : public value
{
  constant m_cst;

public:
  static value_type const vtype;

  value_cst (constant cst, size_t pos)
    : value {vtype, pos}
    , m_cst {cst}
  {}

  value_cst (value_cst const &that) = default;

  constant const &get_constant () const
  { return m_cst; }

  void show (std::ostream &o) const override;
  std::unique_ptr <value> clone () const override;
  cmp_result cmp (value const &that) const override;
};

#endif /* _VALUE_H_ */
