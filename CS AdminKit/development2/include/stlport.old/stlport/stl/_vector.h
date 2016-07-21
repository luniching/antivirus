/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Copyright (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Copyright (c) 1997
 * Moscow Center for SPARC Technology
 *
 * Copyright (c) 1999 
 * Boris Fomitchev
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use or copy this software for any purpose is hereby granted 
 * without fee, provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 *
 */

/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */

#ifndef _STLP_INTERNAL_VECTOR_H
#define _STLP_INTERNAL_VECTOR_H



# ifndef _STLP_INTERNAL_ALGOBASE_H
#  include <stl/_algobase.h>
# endif

# ifndef _STLP_INTERNAL_ALLOC_H
#  include <stl/_alloc.h>
# endif

# ifndef _STLP_INTERNAL_ITERATOR_H
#  include <stl/_iterator.h>
# endif

# ifndef _STLP_INTERNAL_UNINITIALIZED_H
#  include <stl/_uninitialized.h>
# endif

# ifndef _STLP_RANGE_ERRORS_H
#  include <stl/_range_errors.h>
# endif

#ifdef _STLP_DEBUG
#  undef  vector
#  define vector __WORKAROUND_DBG_RENAME(vector)
#endif

_STLP_BEGIN_NAMESPACE 

// The vector base class serves two purposes.  First, its constructor
// and destructor allocate (but don't initialize) storage.  This makes
// exception safety easier.

template <class _Tp, class _Alloc> 
class _Vector_base {
public:

  typedef _Vector_base<_Tp, _Alloc> _Self;
  _STLP_FORCE_ALLOCATORS(_Tp, _Alloc)
  typedef typename _Alloc_traits<_Tp, _Alloc>::allocator_type allocator_type;
  typedef _STLP_alloc_proxy<_Tp*, _Tp, allocator_type> _AllocProxy;

  _Vector_base(const _Alloc& __a)
    : _M_start(0), _M_finish(0), _M_end_of_storage(__a, 0) {
  }
  _Vector_base(size_t __n, const _Alloc& __a)
    : _M_start(0), _M_finish(0), _M_end_of_storage(__a, 0) {
    _M_start = _M_end_of_storage.allocate(__n);
    _M_finish = _M_start;
    _M_end_of_storage._M_data = _M_start + __n;
    _STLP_MPWFIX_TRY _STLP_MPWFIX_CATCH
  }

  _Vector_base(__move_source<_Self> src)
    : _M_start(src.get()._M_start), _M_finish(src.get()._M_finish),
      _M_end_of_storage(_AsMoveSource<_AllocProxy>(src.get()._M_end_of_storage)) {
    //Set the source as empty:
    src.get()._M_start = 0;
    src.get()._M_finish = 0;
    src.get()._M_end_of_storage._M_data = 0;
  }

  ~_Vector_base() { 
    if (_M_start != 0) 
      _M_end_of_storage.deallocate(_M_start, _M_end_of_storage._M_data - _M_start); 
  }

protected:
  void _M_throw_length_error() const;
  void _M_throw_out_of_range() const;

  _Tp* _M_start;
  _Tp* _M_finish;
  _AllocProxy _M_end_of_storage;
};

#ifndef _STLP_DONT_USE_PTR_SPECIALIZATIONS
#  define _VECTOR_IMPL _Vector_impl
#else
#  define _VECTOR_IMPL vector
#endif

template <class _Tp, _STLP_DEFAULT_ALLOCATOR_SELECT(_Tp) >
class _VECTOR_IMPL : protected _Vector_base<_Tp, _Alloc>
#if !defined (_STLP_DEBUG) && defined (_STLP_USE_PARTIAL_SPEC_WORKAROUND)
                     , public __stlport_class<_VECTOR_IMPL<_Tp, _Alloc> >
#endif
{
private:
  typedef _Vector_base<_Tp, _Alloc> _Base;
  typedef _VECTOR_IMPL<_Tp, _Alloc> _Self;
public:
  typedef _Tp value_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type* iterator;
  typedef const value_type* const_iterator;

public:
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef random_access_iterator_tag _Iterator_category;

  _STLP_DECLARE_RANDOM_ACCESS_REVERSE_ITERATORS;
  _STLP_FORCE_ALLOCATORS(_Tp, _Alloc)
  typedef typename _Vector_base<_Tp, _Alloc>::allocator_type allocator_type;

  allocator_type get_allocator() const {
    return _STLP_CONVERT_ALLOCATOR((const allocator_type&)this->_M_end_of_storage, _Tp);
  }
protected:
  typedef typename __type_traits<_Tp>::has_trivial_assignment_operator _TrivialAss;
  typedef typename _TrivialUCopy<_Tp>::_Ret _TrivialUCpy;
  typedef typename __type_traits<_Tp>::has_trivial_copy_constructor _TrivialCpy;
  typedef typename __move_traits<_Tp>::implemented _Movable;

  // handles insertions on overflow
  void _M_insert_overflow(pointer __pos, const _Tp& __x, const __false_type& /*_TrivialCpy*/, 
                          size_type __fill_len, bool __atend = false);
  void _M_insert_overflow(pointer __pos, const _Tp& __x, const __true_type& /*_TrivialCpy*/, 
                          size_type __fill_len, bool __atend = false);
  void _M_range_check(size_type __n) const {
    if (__n >= size_type(this->_M_finish-this->_M_start))
      this->_M_throw_out_of_range();
  }

public:
  iterator begin()             { return this->_M_start; }
  const_iterator begin() const { return this->_M_start; }
  iterator end()               { return this->_M_finish; }
  const_iterator end() const   { return this->_M_finish; }

  reverse_iterator rbegin()              { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const  { return const_reverse_iterator(end()); }
  reverse_iterator rend()                { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const    { return const_reverse_iterator(begin()); }

  size_type size() const        { return size_type(this->_M_finish - this->_M_start); }
  size_type max_size() const {
    size_type __vector_max_size = size_type(-1) / sizeof(_Tp);
    typename allocator_type::size_type __alloc_max_size = this->_M_end_of_storage.max_size();
    return (__alloc_max_size < __vector_max_size)?__alloc_max_size:__vector_max_size;
  }

  size_type capacity() const    { return size_type(this->_M_end_of_storage._M_data - this->_M_start); }
  bool empty() const            { return this->_M_start == this->_M_finish; }

  reference operator[](size_type __n) { return *(begin() + __n); }
  const_reference operator[](size_type __n) const { return *(begin() + __n); }

  reference front()             { return *begin(); }
  const_reference front() const { return *begin(); }
  reference back()              { return *(end() - 1); }
  const_reference back() const  { return *(end() - 1); }

  reference at(size_type __n) { _M_range_check(__n); return (*this)[__n]; }
  const_reference at(size_type __n) const { _M_range_check(__n); return (*this)[__n]; }

  explicit _VECTOR_IMPL(const allocator_type& __a = allocator_type()) : 
    _Vector_base<_Tp, _Alloc>(__a) {}

#if !defined(_STLP_DONT_SUP_DFLT_PARAM)
  explicit _VECTOR_IMPL(size_type __n, const _Tp& __val = _STLP_DEFAULT_CONSTRUCTED(_Tp),
#else
  _VECTOR_IMPL(size_type __n, const _Tp& __val,
#endif /*_STLP_DONT_SUP_DFLT_PARAM*/
         const allocator_type& __a = allocator_type()) 
    : _Vector_base<_Tp, _Alloc>(__n, __a) { 
    this->_M_finish = uninitialized_fill_n(this->_M_start, __n, __val); 
  }

#if defined(_STLP_DONT_SUP_DFLT_PARAM)
  explicit _VECTOR_IMPL(size_type __n)
    : _Vector_base<_Tp, _Alloc>(__n, allocator_type() ) {
    this->_M_finish = uninitialized_fill_n(this->_M_start, __n, _STLP_DEFAULT_CONSTRUCTED(_Tp)); 
  }
#endif /*_STLP_DONT_SUP_DFLT_PARAM*/

  _VECTOR_IMPL(const _Self& __x) 
    : _Vector_base<_Tp, _Alloc>(__x.size(), __x.get_allocator()) { 
    this->_M_finish = __uninitialized_copy(__x.begin(), __x.end(),
                                           this->_M_start, _TrivialUCpy());
  }

  _VECTOR_IMPL(__move_source<_Self> src)
    : _Vector_base<_Tp, _Alloc>(__move_source<_Base>(src.get())) {
  }

#if defined (_STLP_MEMBER_TEMPLATES)

  template <class _Integer>
  void _M_initialize_aux(_Integer __n, _Integer __val,
                         const __true_type& /*_IsIntegral*/) {
    this->_M_start = this->_M_end_of_storage.allocate(__n);
    this->_M_end_of_storage._M_data = this->_M_start + __n; 
    this->_M_finish = uninitialized_fill_n(this->_M_start, __n, __val);
  }

  template <class _InputIterator>
  void _M_initialize_aux(_InputIterator __first, _InputIterator __last,
                         const __false_type& /*_IsIntegral*/) {
    _M_range_initialize(__first, __last, _STLP_ITERATOR_CATEGORY(__first, _InputIterator));
  }

  // Check whether it's an integral type.  If so, it's not an iterator.
  template <class _InputIterator>
  _VECTOR_IMPL(_InputIterator __first, _InputIterator __last,
               const allocator_type& __a _STLP_ALLOCATOR_TYPE_DFL ) :
    _Vector_base<_Tp, _Alloc>(__a) {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_initialize_aux(__first, __last, _Integral());
  }
  
#  ifdef _STLP_NEEDS_EXTRA_TEMPLATE_CONSTRUCTORS
  template <class _InputIterator>
  _VECTOR_IMPL(_InputIterator __first, _InputIterator __last) :
    _Vector_base<_Tp, _Alloc>(allocator_type()) {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_initialize_aux(__first, __last, _Integral());
  }
#  endif /* _STLP_NEEDS_EXTRA_TEMPLATE_CONSTRUCTORS */

#else /* _STLP_MEMBER_TEMPLATES */
  _VECTOR_IMPL(const _Tp* __first, const _Tp* __last,
         const allocator_type& __a = allocator_type())
    : _Vector_base<_Tp, _Alloc>(__last - __first, __a) { 
      this->_M_finish = __uninitialized_copy(__first, __last, this->_M_start, _TrivialUCpy()); 
  }
#endif /* _STLP_MEMBER_TEMPLATES */

  //As the vector container is a back insert container it seems rather logical
  //to destroy elements in reverse order.
  ~_VECTOR_IMPL() { _STLP_STD::_Destroy_Range(rbegin(), rend()); }

  _Self& operator=(const _Self& __x);

  void reserve(size_type __n);

  // assign(), a generalized assignment member function.  Two
  // versions: one that takes a count, and one that takes a range.
  // The range version is a member template, so we dispatch on whether
  // or not the type is an integer.

  void assign(size_type __n, const _Tp& __val) { _M_fill_assign(__n, __val); }
  void _M_fill_assign(size_type __n, const _Tp& __val);
  
#ifdef _STLP_MEMBER_TEMPLATES
  template <class _ForwardIter>
  void _M_assign_aux(_ForwardIter __first, _ForwardIter __last, const forward_iterator_tag &) {
#else
  void assign(const_iterator __first, const_iterator __last) {
    typedef const_iterator _ForwardIter;
#endif
    size_type __len = distance(__first, __last);  
    if (__len > capacity()) {
      iterator __tmp = _M_allocate_and_copy(__len, __first, __last);
      _M_clear();
      _M_set(__tmp, __tmp + __len, __tmp + __len);
    }
    else if (size() >= __len) {
      iterator __new_finish = copy(__first, __last, this->_M_start);
      _STLP_STD::_Destroy_Range(__new_finish, this->_M_finish);
      this->_M_finish = __new_finish;
    }
    else {
      _ForwardIter __mid = __first;
      advance(__mid, size());
      copy(__first, __mid, this->_M_start);
      this->_M_finish = __uninitialized_copy(__mid, __last, this->_M_finish, _TrivialUCpy());
    }
  }

#ifdef _STLP_MEMBER_TEMPLATES
  template <class _InputIter>
  void _M_assign_aux(_InputIter __first, _InputIter __last,
                     const input_iterator_tag &) {
    iterator __cur = begin();
    for ( ; __first != __last && __cur != end(); ++__cur, ++__first)
      *__cur = *__first;
    if (__first == __last)
      erase(__cur, end());
    else
      insert(end(), __first, __last);
  }
  
  template <class _Integer>
  void _M_assign_dispatch(_Integer __n, _Integer __val,
                          const __true_type& /*_IsIntegral*/)
    { _M_fill_assign(__n, __val); }

  template <class _InputIter>
  void _M_assign_dispatch(_InputIter __first, _InputIter __last,
                          const __false_type& /*_IsIntegral*/)
    { _M_assign_aux(__first, __last, _STLP_ITERATOR_CATEGORY(__first, _InputIter)); }

  template <class _InputIterator>
  void assign(_InputIterator __first, _InputIterator __last) {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_assign_dispatch(__first, __last, _Integral());
  }
#endif /* _STLP_MEMBER_TEMPLATES */

#if !defined(_STLP_DONT_SUP_DFLT_PARAM) && !defined(_STLP_NO_ANACHRONISMS)
  void push_back(const _Tp& __x = _STLP_DEFAULT_CONSTRUCTED(_Tp)) {
#else
  void push_back(const _Tp& __x) {
#endif /*!_STLP_DONT_SUP_DFLT_PARAM && !_STLP_NO_ANACHRONISMS*/
    if (this->_M_finish != this->_M_end_of_storage._M_data) {
      _Copy_Construct(this->_M_finish, __x);
      ++this->_M_finish;
    }
    else
      _M_insert_overflow(this->_M_finish, __x, _TrivialCpy(), 1UL, true);
  }

#if !defined(_STLP_DONT_SUP_DFLT_PARAM) && !defined(_STLP_NO_ANACHRONISMS)
  iterator insert(iterator __pos, const _Tp& __x = _STLP_DEFAULT_CONSTRUCTED(_Tp));
#else
  iterator insert(iterator __pos, const _Tp& __x);
#endif /*!_STLP_DONT_SUP_DFLT_PARAM && !_STLP_NO_ANACHRONISMS*/

#if defined(_STLP_DONT_SUP_DFLT_PARAM) && !defined(_STLP_NO_ANACHRONISMS)
  void push_back() { push_back(_STLP_DEFAULT_CONSTRUCTED(_Tp)); }
  iterator insert(iterator __pos) { return insert(__pos, _STLP_DEFAULT_CONSTRUCTED(_Tp)); }
# endif /*_STLP_DONT_SUP_DFLT_PARAM && !_STLP_NO_ANACHRONISMS*/

  void swap(_Self& __x) {
    _STLP_STD::swap(this->_M_start, __x._M_start);
    _STLP_STD::swap(this->_M_finish, __x._M_finish);
    _STLP_STD::swap(this->_M_end_of_storage, __x._M_end_of_storage);
  }

private:
  void _M_fill_insert_aux (iterator __pos, size_type __n, const _Tp& __x, const __true_type& /*_Movable*/);
  void _M_fill_insert_aux (iterator __pos, size_type __n, const _Tp& __x, const __false_type& /*_Movable*/);
  void _M_fill_insert (iterator __pos, size_type __n, const _Tp& __x);

#if defined (_STLP_MEMBER_TEMPLATES)
  template <class _ForwardIterator>
  void _M_range_insert_realloc(iterator __pos,
                               _ForwardIterator __first, _ForwardIterator __last,
                               size_type __n) {
#else
  void _M_range_insert_realloc(iterator __pos,
                               const_iterator __first, const_iterator __last,
                               size_type __n) {
#endif /* _STLP_MEMBER_TEMPLATES */
    const size_type __old_size = size();
    const size_type __len = __old_size + (max)(__old_size, __n);
    pointer __new_start = this->_M_end_of_storage.allocate(__len);
    pointer __new_finish = __new_start;
    _STLP_TRY {
      __new_finish = __uninitialized_move(this->_M_start, __pos, __new_start, _TrivialUCpy(), _Movable());
      __new_finish = __uninitialized_copy(__first, __last, __new_finish, _TrivialUCpy());
      __new_finish = __uninitialized_move(__pos, this->_M_finish, __new_finish, _TrivialUCpy(), _Movable());
    }
    _STLP_UNWIND((_STLP_STD::_Destroy_Range(__new_start,__new_finish), 
                  this->_M_end_of_storage.deallocate(__new_start,__len)))
    _M_clear_after_move();
    _M_set(__new_start, __new_finish, __new_start + __len);
  }

#if defined (_STLP_MEMBER_TEMPLATES)
  template <class _ForwardIterator>
  void _M_range_insert_aux(iterator __pos,
                           _ForwardIterator __first, _ForwardIterator __last,
                           size_type __n, const __true_type& /*_Movable*/) {
#else
  void _M_range_insert_aux(iterator __pos,
                           const_iterator __first, const_iterator __last,
                           size_type __n, const __true_type& /*_Movable*/) {
#endif /* _STLP_MEMBER_TEMPLATES */
    iterator __src = this->_M_finish;
    iterator __dst = __src + __n;
    for (; __src != __pos; --__dst, --__src) {
      _STLP_STD::_Move_Construct(__dst, *__src);
      _STLP_STD::_Destroy_Moved(__src);
    }
    __uninitialized_copy(__first, __last, __pos, _TrivialUCpy());
    this->_M_finish += __n;
  }

#if defined (_STLP_MEMBER_TEMPLATES)
  template <class _ForwardIterator>
  void _M_range_insert_aux(iterator __pos,
                           _ForwardIterator __first, _ForwardIterator __last,
#else
  void _M_range_insert_aux(iterator __pos,
                           const_iterator __first, const_iterator __last,
#endif /* _STLP_MEMBER_TEMPLATES */
                           size_type __n, const __false_type& /*_Movable*/) {
    const size_type __elems_after = this->_M_finish - __pos;
    pointer __old_finish = this->_M_finish;
    if (__elems_after > __n) {
      __uninitialized_copy(this->_M_finish - __n, this->_M_finish, this->_M_finish, _TrivialUCpy());
      this->_M_finish += __n;
      __copy_backward_ptrs(__pos, __old_finish - __n, __old_finish, _TrivialAss());
      copy(__first, __last, __pos);
    }
    else {
# if defined ( _STLP_MEMBER_TEMPLATES )
      _ForwardIterator __mid = __first;
      advance(__mid, __elems_after);
# else
      const_pointer __mid = __first + __elems_after;
# endif
      __uninitialized_copy(__mid, __last, this->_M_finish, _TrivialUCpy());
      this->_M_finish += __n - __elems_after;
      __uninitialized_copy(__pos, __old_finish, this->_M_finish, _TrivialUCpy());
      this->_M_finish += __elems_after;
      copy(__first, __mid, __pos);
    } /* elems_after */
  }


#if defined ( _STLP_MEMBER_TEMPLATES)
  template <class _Integer>
  void _M_insert_dispatch(iterator __pos, _Integer __n, _Integer __val,
                          const __true_type&) {
    _M_fill_insert(__pos, (size_type) __n, (_Tp) __val);
  }

  template <class _InputIterator>
  void _M_insert_dispatch(iterator __pos,
                          _InputIterator __first, _InputIterator __last,
                          const __false_type&) {
    _M_range_insert(__pos, __first, __last, _STLP_ITERATOR_CATEGORY(__first, _InputIterator));
  }

public:
  // Check whether it's an integral type.  If so, it's not an iterator.
  template <class _InputIterator>
  void insert(iterator __pos, _InputIterator __first, _InputIterator __last) {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_insert_dispatch(__pos, __first, __last, _Integral());
  }
  
private:
  template <class _InputIterator>
  void _M_range_insert(iterator __pos, 
                       _InputIterator __first, _InputIterator __last,
                       const input_iterator_tag &) {
    for ( ; __first != __last; ++__first) {
      __pos = insert(__pos, *__first);
      ++__pos;
    }
  }

  template <class _ForwardIterator>
  void _M_range_insert(iterator __pos,
                       _ForwardIterator __first, _ForwardIterator __last,
                       const forward_iterator_tag &) {
#else /* _STLP_MEMBER_TEMPLATES */
public:
  void insert(iterator __pos,
              const_iterator __first, const_iterator __last) {
#endif /* _STLP_MEMBER_TEMPLATES */
    if (__first != __last) {
      size_type __n = distance(__first, __last);

      if (size_type(this->_M_end_of_storage._M_data - this->_M_finish) >= __n) {
        _M_range_insert_aux(__pos, __first, __last, __n, _Movable());
      }
      else {
        _M_range_insert_realloc(__pos, __first, __last, __n);
      }
    }
  }
  
public:
  void insert (iterator __pos, size_type __n, const _Tp& __x)
  { _M_fill_insert(__pos, __n, __x); }
  
  void pop_back() {
    --this->_M_finish;
    _STLP_STD::_Destroy(this->_M_finish);
  }

protected:
  iterator _M_erase(iterator __pos, const __true_type& /*_Movable*/) {
    _STLP_STD::_Destroy(__pos);
    iterator __dst = __pos, __src = __dst + 1;
    iterator __end = end();
    for (; __src != __end; ++__dst, ++__src) {
      _STLP_STD::_Move_Construct(__dst, *__src);
      _STLP_STD::_Destroy_Moved(__src);
    }
    this->_M_finish = __dst;
    return __pos;
  }
  iterator _M_erase(iterator __pos, const __false_type& /*_Movable*/) {
    if (__pos + 1 != end())
      __copy_ptrs(__pos + 1, this->_M_finish, __pos, _TrivialAss());
    --this->_M_finish;
    _STLP_STD::_Destroy(this->_M_finish);
    return __pos;
  }
  iterator _M_erase(iterator __first, iterator __last, const __true_type& /*_Movable*/) {
    iterator __dst = __first, __src = __last;
    iterator __end = end();
    for (; __dst != __last && __src != __end; ++__dst, ++__src) {
      _STLP_STD::_Destroy(__dst);
      _STLP_STD::_Move_Construct(__dst, *__src);
    }
    for (; __src != __end; ++__dst, ++__src) {
      _STLP_STD::_Destroy_Moved(__dst);
      _STLP_STD::_Move_Construct(__dst, *__src);
    }
    _STLP_STD::_Destroy_Moved_Range(__dst, __end);
    this->_M_finish = __dst;
    return __first;
  }
  iterator _M_erase(iterator __first, iterator __last, const __false_type& /*_Movable*/) {
    pointer __i = __copy_ptrs(__last, this->_M_finish, __first, _TrivialAss());
    _STLP_STD::_Destroy_Range(__i, this->_M_finish);
    this->_M_finish = __i;
    return __first;
  }

public:
  iterator erase(iterator __pos) {
    return _M_erase(__pos, _Movable());
  }
  iterator erase(iterator __first, iterator __last) {
    return _M_erase(__first, __last, _Movable());
  }

#if !defined(_STLP_DONT_SUP_DFLT_PARAM)
  void resize(size_type __new_size, const _Tp& __x = _STLP_DEFAULT_CONSTRUCTED(_Tp)) {
#else
  void resize(size_type __new_size, const _Tp& __x) {
#endif /*_STLP_DONT_SUP_DFLT_PARAM*/
    if (__new_size < size()) 
      erase(begin() + __new_size, end());
    else
      insert(end(), __new_size - size(), __x);
  }

#if defined(_STLP_DONT_SUP_DFLT_PARAM)
  void resize(size_type __new_size) { resize(__new_size, _STLP_DEFAULT_CONSTRUCTED(_Tp)); }
#endif /*_STLP_DONT_SUP_DFLT_PARAM*/

  void clear() { 
    erase(begin(), end());
  }

protected:

  void _M_clear() {
    _STLP_STD::_Destroy_Range(rbegin(), rend());
    this->_M_end_of_storage.deallocate(this->_M_start, this->_M_end_of_storage._M_data - this->_M_start);
  }

  void _M_clear_after_move() {
    _STLP_STD::_Destroy_Moved_Range(rbegin(), rend());
    this->_M_end_of_storage.deallocate(this->_M_start, this->_M_end_of_storage._M_data - this->_M_start);
  }

  void _M_set(pointer __s, pointer __f, pointer __e) {
    this->_M_start = __s;
    this->_M_finish = __f;
    this->_M_end_of_storage._M_data = __e;
  }

#ifdef _STLP_MEMBER_TEMPLATES
  template <class _ForwardIterator>
  pointer _M_allocate_and_copy(size_type __n, _ForwardIterator __first, 
                               _ForwardIterator __last)
#else /* _STLP_MEMBER_TEMPLATES */
  pointer _M_allocate_and_copy(size_type __n, const_pointer __first, 
                               const_pointer __last)
#endif /* _STLP_MEMBER_TEMPLATES */
  {
    pointer __result = this->_M_end_of_storage.allocate(__n);
    _STLP_TRY {
#if !defined(__MRC__)  //*TY 12/17/2000 - added workaround for MrCpp. it confuses on nested try/catch block
      __uninitialized_copy(__first, __last, __result, _TrivialUCpy());
#else
      uninitialized_copy(__first, __last, __result);
#endif
      return __result;
    }
    _STLP_UNWIND(this->_M_end_of_storage.deallocate(__result, __n))
    _STLP_RET_AFTER_THROW(__result)
  }


#ifdef _STLP_MEMBER_TEMPLATES
  template <class _InputIterator>
  void _M_range_initialize(_InputIterator __first,  
                           _InputIterator __last, const input_iterator_tag &) {
    for ( ; __first != __last; ++__first)
      push_back(*__first);
  }
  // This function is only called by the constructor. 
  template <class _ForwardIterator>
  void _M_range_initialize(_ForwardIterator __first,
                           _ForwardIterator __last, const forward_iterator_tag &) {
    size_type __n = distance(__first, __last);
    this->_M_start = this->_M_end_of_storage.allocate(__n);
    this->_M_end_of_storage._M_data = this->_M_start + __n;
    this->_M_finish = __uninitialized_copy(__first, __last, this->_M_start, _TrivialUCpy());
  }
  
#endif /* _STLP_MEMBER_TEMPLATES */
};

# ifndef _STLP_DONT_USE_PTR_SPECIALIZATIONS
#  include <stl/pointers/_vector.h>
# endif /* _STLP_DONT_USE_PTR_SPECIALIZATIONS */

# define _STLP_TEMPLATE_HEADER    template <class _Tp, class _Alloc>
# define _STLP_TEMPLATE_CONTAINER vector<_Tp, _Alloc>
# include <stl/_relops_cont.h>
# undef _STLP_TEMPLATE_CONTAINER
# undef _STLP_TEMPLATE_HEADER

#ifdef _STLP_CLASS_PARTIAL_SPECIALIZATION
template <class _Tp, class _Alloc>
struct __move_traits<vector<_Tp, _Alloc> > {
  typedef __true_type implemented;
  //Completness depends on the allocator:
  typedef typename __move_traits<_Alloc>::complete complete;
};
#endif /* _STLP_CLASS_PARTIAL_SPECIALIZATION */


# if defined (_STLP_USE_TEMPLATE_EXPORT)
_STLP_EXPORT_TEMPLATE_CLASS allocator<void*>;
_STLP_EXPORT_TEMPLATE_CLASS _STLP_alloc_proxy<void**, void*, allocator<void*> >;
_STLP_EXPORT_TEMPLATE_CLASS _Vector_base<void*,allocator<void*> >;

# ifndef _STLP_DONT_USE_PTR_SPECIALIZATIONS
_STLP_EXPORT_TEMPLATE_CLASS _Vector_impl<void*,allocator<void*> >;
# endif

_STLP_EXPORT_TEMPLATE_CLASS vector<void*,allocator<void*> >;
# endif

#ifdef _STLP_DEBUG
# undef  vector
#endif
// #  undef  __vector__
// #  define __vector__ __WORKAROUND_RENAME(vector)

_STLP_END_NAMESPACE

# if !defined (_STLP_LINK_TIME_INSTANTIATION)
#  include <stl/_vector.c>
# endif

# undef _VECTOR_IMPL

#ifndef _STLP_INTERNAL_BVECTOR_H
# include <stl/_bvector.h>
#endif

# if defined (_STLP_DEBUG)
#  include <stl/debug/_vector.h>
# endif

# if defined (_STLP_USE_WRAPPER_FOR_ALLOC_PARAM)
#  include <stl/wrappers/_vector.h>
# endif

#endif /* _STLP_VECTOR_H */

// Local Variables:
// mode:C++
// End:
