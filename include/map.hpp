//
// Created by 김민규 on 2023/01/23.
//


#ifndef FT_CONTAINER_MAP_HPP

#include <memory>
#include <functional> // for std::less
#include "iterator.hpp"
#include "__Left_Leaning_RedBlack.hpp"
#include "__config.hpp"


FT_BEGIN_PRIVATE_NAMESPACE

// ---------------------------------------------------------------
// |                                                             |
// |                   map_iterator_implementation               |
// |                                                             |
// ---------------------------------------------------------------
// * [map_iterator를 구현한 이유]
// * __tree_iterator의 경우 node 포인터입니다.
// * map에서 __tree_iterator를 그대로 사용할 경우, iterator의 dereferencing 결과는 tree_node입니다.
// * 이러면 안됩니다.
// * map iterator의 dereference 는 pair여야 합니다.
// * 바로 이점 때문에 구현하였습니다.
// * 그럼 __tree_iterator에서 key를 반환하도록 하면 될 것 같지만,
// * 그렇게 할 경우 __tree_base (ex. Red Black Tree) 의 로직이 이 node iterator를 기반으로 하고 있어서
// * 수정하는 순간 펑 ! 하고 터집니다.

template <class _TreeIterator>
class __map_iterator
{

private:
    typedef typename _TreeIterator::node_type                    _node_type;   //  RbNode

private: // data member
// * -------------------------------------------
    _TreeIterator __i_;
// * -------------------------------------------

public:
    typedef std::bidirectional_iterator_tag                      iterator_category;
    typedef typename _node_type::key_type                        value_type;        // pair
    typedef typename _TreeIterator::difference_type              difference_type;
    typedef value_type&                                          reference;
    typedef typename _node_type::key_type_pointer                pointer;  // const pair *

    __map_iterator() _NOEXCEPT
    {}

    __map_iterator(_TreeIterator __i) _NOEXCEPT
        : __i_(__i)
    {}

    reference operator*() const
    { return (__i_->key); }

    pointer operator->() const
    { return &(__i_->key); }

    __map_iterator& operator++() {++__i_; return *this;}

    __map_iterator operator++(int)
    {
        __map_iterator __t(*this);
        ++(*this);
        return __t;
    }

    __map_iterator& operator--() {--__i_; return *this;}

    __map_iterator operator--(int)
    {
        __map_iterator __t(*this);
        --(*this);
        return __t;
    }

    friend
    bool operator==(const __map_iterator& __x, const __map_iterator& __y)
    {return __x.__i_ == __y.__i_;}

    friend
    bool operator!=(const __map_iterator& __x, const __map_iterator& __y)
    {return __x.__i_ != __y.__i_;}
};

template <class _TreeIterator>
class __map_const_iterator
{

private:
    typedef typename _TreeIterator::node_type                    _node_type;

private: // data member
// * -------------------------------------------
    _TreeIterator __i_;
// * -------------------------------------------

public:
    typedef std::bidirectional_iterator_tag                      iterator_category;
    typedef typename _node_type::key_type                        value_type;        // pair
    typedef typename _TreeIterator::difference_type              difference_type;
    typedef value_type&                                          reference;
    typedef typename _node_type::const_key_type_pointer          pointer;  // const pair *

    __map_const_iterator() _NOEXCEPT
    {}

    __map_const_iterator(_TreeIterator __i) _NOEXCEPT
            : __i_(__i)
    {}

    reference operator*() const
    { return (__i_->key); }

    pointer operator->() const
    { return &(__i_->key); }

    __map_const_iterator& operator++() {++__i_; return *this;}

    __map_const_iterator operator++(int)
    {
        __map_const_iterator __t(*this);
        ++(*this);
        return __t;
    }

    __map_const_iterator& operator--() {--__i_; return *this;}

    __map_const_iterator operator--(int)
    {
        __map_const_iterator __t(*this);
        --(*this);
        return __t;
    }

    friend
    bool operator==(const __map_const_iterator& __x, const __map_const_iterator& __y)
    {return __x.__i_ == __y.__i_;}

    friend
    bool operator!=(const __map_const_iterator& __x, const __map_const_iterator& __y)
    {return __x.__i_ != __y.__i_;}
};

FT_END_PRIVATE_NAMESPACE











FT_BEGIN_GLOBAL_NAMESPACE

// ---------------------------------------------------------------
// |                                                             |
// |                   map implementation                        |
// |                                                             |
// ---------------------------------------------------------------
// * map은 LeftLeaningRedBlack class의 adapter class이며, 내부 로직은 모두 __tree__ 멤버 데이터를 통해 이뤄집니다.

template <class Key, class Value, class Compare = std::less<Key>,
        class Allocator = std::allocator<FT::pair<const Key, Value> > >
class FT_TEMPLATE_VIS map
{
public:
    // Member types:
    typedef Key                                         key_type;         // 각 map 요소에 저장된 정렬 키에 대한 typedef
    typedef Value                                       mapped_type;      // 각 map 요소에 저장된 데이터에 대한 typedef
    typedef Compare                                     key_compare;      // map의 두 요소간 상대적 순서를 결정하는 두 정렬 키를 비교할 수 있는 함수 개체에 대한 typedef
    typedef Allocator                                   allocator_type;   // map 개체를 위한 allocator 클래스의 typedef

    typedef FT::pair<const key_type, mapped_type>       value_type;       // map의 요소로 저장된 개체의 형식에 대한 typedef
    typedef value_type&                                 reference;        // map에 저장된 요소에 대한 참조의 typedef
    typedef const value_type&                           const_reference;  // 읽기 및 const 작업을 수행하기 위해 const 맵에 저장된 요소에 대한 참조에 대한 typedef

    // map은 둘다 const이다. 왜냐면 Key에 const가 무조건 붙어서 들어가기 때문이다.
    typedef typename allocator_type::pointer            pointer;          // map의 const 요소를 가리키는 포인터에 대한 typedef
    typedef typename allocator_type::const_pointer      const_pointer;    // map의 const 요소를 가리키는 포인터에 대한 typedef
    typedef typename allocator_type::size_type          size_type;        // map의 요소 수에 대한 부호 없는 정수의 typedef
    typedef typename allocator_type::difference_type    difference_type;  // 반복기가 가리키는 요소 사이의 범위에 있는 map의 요소 개수에 대한 부호 있는 정수 typedef

public:
    // Member classes: a function object for map
    // Ref : https://en.cppreference.com/w/cpp/container/map/value_compare
    class value_compare : public std::binary_function<value_type, value_type, bool>
    {
        friend class map<key_type, mapped_type, key_compare, allocator_type>;

    protected:
        Compare comp;
        value_compare(Compare c) : comp(c) {}

    public:
        bool operator()(const value_type &x, const value_type &y) const
        {
            return (comp(x.first, y.first));
        }
    };

private:
    // * 여기서 pair type 정렬을 위한 value_compare를 넣어준다.
    typedef _PRIVATE::LeftLeaningRedBlack<value_type, value_compare>        _map_base;
    typedef FT::map<Key, Value, Compare, Allocator>                         _map_type;

public:
    // WARN: map base's iterator is a pointer to a tree-Node.
    // need to convert.
    typedef _PRIVATE::__map_iterator<typename _map_base::iterator>                    iterator;
    typedef _PRIVATE::__map_const_iterator<typename _map_base::const_iterator>        const_iterator;
    typedef typename FT::reverse_iterator<iterator>                         reverse_iterator;
    typedef typename FT::reverse_iterator<const_iterator>                   const_reverse_iterator;


private: // Member data:
// * -------------------------------------------------
    _map_base   __tree__;
// * -------------------------------------------------


public: // constructor & destructor.

    explicit map(const key_compare& comp = key_compare(), const allocator_type& a = allocator_type())
            : __tree__(comp, a)
    {}


    template <class InputIterator>
    map(InputIterator first, InputIterator last, const key_compare& comp = key_compare())
            : __tree__(comp)
    {
        this->insert(first, last);
    }

    template <class InputIterator>
    map(InputIterator first, InputIterator last, const key_compare& comp, const allocator_type& a)
            : __tree__(comp, a)
    {
        this->insert(first, last);
    }

    map(const _map_type& m)
            : __tree__(m.__tree__)
    {}

    explicit map(const allocator_type& a)
            : __tree__(a)
    {}

    map(const _map_type& m, const allocator_type& a)
            : __tree__(m.__tree__, a)
    {}

    ~map()
    {
        // ...
    }

    map& operator=(const _map_type& m)
    {
        __tree__ = m.__tree__;
    }

    allocator_type get_allocator() const _NOEXCEPT
    {
        return allocator_type();
    }

public:
    // iterators:
    iterator begin() _NOEXCEPT
    { return __tree__.begin(); }

    const_iterator begin() const _NOEXCEPT
    { return __tree__.begin(); }

    iterator end() _NOEXCEPT
    { return __tree__.end(); }

    const_iterator end() const _NOEXCEPT
    { return __tree__.end(); }

    reverse_iterator rbegin() _NOEXCEPT
    { return reverse_iterator(end()); }

    const_reverse_iterator rbegin() const _NOEXCEPT
    { return const_reverse_iterator(end()); }

    reverse_iterator rend() _NOEXCEPT
    { return reverse_iterator(begin()); }

    const_reverse_iterator rend() const _NOEXCEPT
    { return const_reverse_iterator(begin()); }

public:
    // capacity:
    bool empty() const _NOEXCEPT
    { return __tree__.isEmpty(); }

    size_type size() const _NOEXCEPT
    { return __tree__.size(); }

    size_type max_size() const _NOEXCEPT
    { return __tree__.max_size(); }

public:
    // element access:

    // Returns a reference to the value that is mapped to a key -
    // equivalent to [k], performing an insertion if such key does not already exist.
    // Inserts value_type(key, T()) if the key does not exist.
    mapped_type& operator[](const key_type& k) _NOEXCEPT
    {
        // [ value_type(...) ] == [ FT::pair<const key_type, mapped_type >(...) ]
        value_type* pair_ptr = __tree__.getAddressOfKey(value_type(k, mapped_type()));
        if (pair_ptr == NULL) { // if not found, perform an insertion.
            __tree__.put(value_type(k, mapped_type()));
        }
        // then, return reference of mapped_type.
        pair_ptr = __tree__.getAddressOfKey(value_type(k, mapped_type()));
        return (pair_ptr->second);
    }

    // 왜 이렇게 하나면.. __tree__ 의 key 자체가 pair 타입이기 때문이다. pair끼리 비교해서 값을 찾아낸다.
    mapped_type& at(const key_type& k)
    {
        value_type* pair_ptr = __tree__.getAddressOfKey(value_type(k, mapped_type()));

        if (pair_ptr == NULL) { // key not found
            throw std::out_of_range("map");
        }
        return (pair_ptr->second);
    }

    const mapped_type& at(const key_type& k) const
    {
        value_type* pair_ptr = __tree__.getAddressOfKey(value_type(k, mapped_type()));

        if (pair_ptr == NULL) { // key not found
            throw std::out_of_range("map");
        }
        return (pair_ptr->second);
    }

public:
    // modifiers:

     /**
     Inserts element(s) into the container, if the container
     doesn't already contain an element with an equivalent key.
     @Return a pair consisting of an iterator to the inserted element
             (or to the element that prevented the insertion)
             and a bool value set to true if and only if the insertion took place.
     */
    FT::pair<iterator, bool> insert(const value_type& v) _NOEXCEPT
    {
        // 1. check if it has data.
        value_type* pair_ptr = __tree__.getAddressOfKey(v);
        if (pair_ptr != NULL) { // if pair already exists.
            return FT::make_pair( iterator(pair_ptr), false );
        } else { // if pair doesn't exist, insert data.
            __tree__.put(v);
            pair_ptr = __tree__.getAddressOfKey(v);
            return FT::make_pair( iterator(pair_ptr), true );
        }
    }

    iterator insert(const_iterator position, const value_type& v)
    {

    }

    template <class InputIterator>
    void insert(InputIterator first, InputIterator last)
    {
        __tree__.put(first, last);
    }

    void clear()
    {
        __tree__.clear();
    }

    void erase( iterator pos );
    void erase( iterator first, iterator last );
    size_type erase( const Key& key );
    void swap( map& other );

public:
    // Lookup:
    size_type count( const Key& key ) const;
    iterator find( const Key& key );
    const_iterator find( const Key& key ) const;

    FT::pair<iterator,iterator> equal_range( const Key& key );
    FT::pair<const_iterator,const_iterator> equal_range( const Key& key ) const;

    iterator lower_bound( const Key& key );
    const_iterator lower_bound( const Key& key ) const;

    iterator upper_bound( const Key& key );
    const_iterator upper_bound( const Key& key ) const;

public:
    // Observers:
    key_compare key_comp() const;
    value_compare value_comp() const;

};

// Non-member functions:
template <class Key, class T, class Compare, class Allocator>
bool
operator==(const map<Key, T, Compare, Allocator>& x,
           const map<Key, T, Compare, Allocator>& y);

template <class Key, class T, class Compare, class Allocator>
bool
operator< (const map<Key, T, Compare, Allocator>& x,
           const map<Key, T, Compare, Allocator>& y);

template <class Key, class T, class Compare, class Allocator>
bool
operator!=(const map<Key, T, Compare, Allocator>& x,
           const map<Key, T, Compare, Allocator>& y);

template <class Key, class T, class Compare, class Allocator>
bool
operator> (const map<Key, T, Compare, Allocator>& x,
           const map<Key, T, Compare, Allocator>& y);

template <class Key, class T, class Compare, class Allocator>
bool
operator>=(const map<Key, T, Compare, Allocator>& x,
           const map<Key, T, Compare, Allocator>& y);

template <class Key, class T, class Compare, class Allocator>
bool
operator<=(const map<Key, T, Compare, Allocator>& x,
           const map<Key, T, Compare, Allocator>& y);

// specialized algorithms:
template <class Key, class T, class Compare, class Allocator>
void
swap(map<Key, T, Compare, Allocator>& x, map<Key, T, Compare, Allocator>& y); /* (C++17) _NOEXCEPT_(_NOEXCEPT_(x.swap(y))) */

FT_END_GLOBAL_NAMESPACE

#define FT_CONTAINER_MAP_HPP

#endif //FT_CONTAINER_MAP_HPP
