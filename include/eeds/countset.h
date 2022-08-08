#pragma once

#include <concepts>
#include <functional>
#include <initializer_list>
#include <limits>
#include <type_traits>

namespace eeds {

template<class It>
using iter_value_t = typename std::iterator_traits<It>::value_type;

template<class A, class T>
concept Allocator = requires(A&& a, std::size_t n, T* p) {
                      { a.allocate(n) } -> std::same_as<T*>;
                      { a.deallocate(p, n) } noexcept;
                    };

template<class C, class T>
concept Comparator = requires(const C& c, const T& a) {
                       { c(a, a) } noexcept -> std::same_as<bool>;
                     };

template<class R>
concept RandomNumberGen = requires(R& rng) {
                            { rng() } noexcept -> std::same_as<std::size_t>;
                          };

template<class T, Comparator<T>, Allocator<T>, RandomNumberGen>
struct countset;

enum class iterator_dir {
  fwd,
  rev,
};

template<class T>
struct countset_node {
  countset_node(const countset_node&) = delete;
  countset_node(countset_node&&) = delete;
  countset_node& operator=(const countset_node&) = delete;
  countset_node& operator=(countset_node&&) = delete;

  [[nodiscard]] const T& value() const noexcept {
    return k;
  }

  [[nodiscard]] std::size_t get_priority() const noexcept {
    return priority;
  }

  [[nodiscard]] std::size_t get_count() const noexcept {
    return count;
  }

  [[nodiscard]] const countset_node* get_left() const noexcept {
    return left;
  }

  [[nodiscard]] const countset_node* get_right() const noexcept {
    return right;
  }

  [[nodiscard]] const countset_node* get_parent() const noexcept {
    return parent;
  }

  [[nodiscard]] std::size_t index() const noexcept {
    auto node = this;
    auto total = count_of(node->left);
    while (node->parent != nullptr) {
      if (node == node->parent->right) {
        total += count_of(node->parent->left) + 1;
      }
      node = node->parent;
    }
    return total;
  }

  [[nodiscard]] const countset_node* leftmost() const noexcept {
    auto node = this;
    while (node->left != nullptr) {
      node = node->left;
    }
    return node;
  }

  [[nodiscard]] const countset_node* rightmost() const noexcept {
    auto node = this;
    while (node->right != nullptr) {
      node = node->right;
    }
    return node;
  }

  [[nodiscard]] const countset_node* next() const noexcept {
    if (right != nullptr) {
      return right->leftmost();
    }
    auto node = this;
    while (true) {
      if (node->parent == nullptr) {
        return nullptr;
      }
      if (node == node->parent->left) {
        return node->parent;
      }
      node = node->parent;
    }
  }

  [[nodiscard]] const countset_node* prev() const noexcept {
    if (left != nullptr) {
      return left->rightmost();
    }
    auto node = this;
    while (true) {
      if (node->parent == nullptr) {
        return nullptr;
      }
      if (node == node->parent->right) {
        return node->parent;
      }
      node = node->parent;
    }
  }

  [[nodiscard]] const countset_node*
      lower_bound(const auto& key,
                  const Comparator<T> auto& cmp) const noexcept {
    if (cmp(key, k)) {
      if (left == nullptr) {
        return this;
      }
      auto left_value = left->lower_bound(key, cmp);
      if (left_value == nullptr) {
        return this;
      }
      return left_value;
    }
    if (right == nullptr) {
      return nullptr;
    }
    return right->lower_bound(key, cmp);
  }

  [[nodiscard]] const countset_node*
      find(const auto& key, const Comparator<T> auto& cmp) const noexcept {
    auto node = this;
    do {
      if (cmp(key, node->k)) {
        node = node->left;
      } else if (cmp(node->k, key)) {
        node = node->right;
      } else {
        while (node->left != nullptr && !cmp(node->left->k, key)) {
          node = node->left;
        }
        break;
      }
    } while (node != nullptr);
    return node;
  }

private:
  template<class... Args>
  explicit countset_node(std::size_t priority, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<T, Args&&...>)
          : priority(priority), k(std::forward<Args>(args)...) {
  }

  // Clone
  explicit countset_node(const countset_node* other) noexcept(
      std::is_nothrow_copy_constructible_v<T>)
          : count(other->count), priority(other->priority), k(other->k) {
  }

  static std::size_t count_of(const countset_node* node) noexcept {
    return node == nullptr ? 0 : node->count;
  }

  static countset_node* merge(countset_node* left,
                              countset_node* right) noexcept {
    if (left == nullptr) {
      return right;
    }
    if (right == nullptr) {
      return left;
    }
    if (left->priority > right->priority) {
      left->count += right->count;
      left->right = merge(left->right, right);
      if (left->right != nullptr) {
        left->right->parent = left;
      }
      return left;
    }
    right->count += left->count;
    right->left = merge(left, right->left);
    if (right->left != nullptr) {
      right->left->parent = right;
    }
    return right;
  }

  static std::tuple<countset_node*, countset_node*, countset_node*>
      split(countset_node* root,
            const auto& key,
            const Comparator<T> auto& cmp) noexcept {
    if (root == nullptr) {
      return {nullptr, nullptr, nullptr};
    }
    if (cmp(key, root->k)) {
      const auto count_left = count_of(root->left);
      const auto [conflict, left, right] = split(root->left, key, cmp);
      if (conflict != nullptr) {
        return {conflict, nullptr, nullptr};
      }
      root->count -= count_left;
      root->left = right;
      if (right != nullptr) {
        right->parent = root;
        root->count += right->count;
      }
      return {nullptr, left, root};
    }
    if (cmp(root->k, key)) {
      const auto count_right = count_of(root->right);
      const auto [conflict, left, right] = split(root->right, key, cmp);
      if (conflict != nullptr) {
        return {conflict, nullptr, nullptr};
      }
      root->count -= count_right;
      root->right = left;
      if (left != nullptr) {
        left->parent = root;
        root->count += left->count;
      }
      return {nullptr, root, right};
    }
    return {root, nullptr, nullptr};
  }

  static countset_node* insert(countset_node*& root,
                               countset_node* node,
                               const Comparator<T> auto& cmp) noexcept {
    if (root == nullptr) {
      root = node;
      return nullptr;
    }
    if (node->priority > root->priority) {
      const auto [conflict, left, right] = split(root, node->k, cmp);
      if (conflict != nullptr) {
        return conflict;
      }
      if (left != nullptr) {
        node->left = left;
        node->count += node->left->count;
        node->left->parent = node;
      }
      if (right != nullptr) {
        node->right = right;
        node->count += node->right->count;
        node->right->parent = node;
      }
      root = node;
      return nullptr;
    }
    if (cmp(node->k, root->k)) {
      auto conflict = insert(root->left, node, cmp);
      if (conflict == nullptr) {
        root->left->parent = root;
        root->count += 1;
      }
      return conflict;
    }
    if (cmp(root->k, node->k)) {
      auto conflict = insert(root->right, node, cmp);
      if (conflict == nullptr) {
        root->right->parent = root;
        root->count += 1;
      }
      return conflict;
    }
    return root;
  }

  static void erase(countset_node*& root, countset_node* node) noexcept {
    if (node == root) {
      root = merge(node->left, node->right);
      if (root != nullptr) {
        root->parent = nullptr;
      }
      return;
    }
    if (node == node->parent->left) {
      node->parent->left = merge(node->left, node->right);
      if (node->parent->left != nullptr) {
        node->parent->left->parent = node->parent;
      }
    } else {
      node->parent->right = merge(node->left, node->right);
      if (node->parent->right != nullptr) {
        node->parent->right->parent = node->parent;
      }
    }
    auto n = node->parent;
    do {
      n->count -= 1;
      n = n->parent;
    } while (n != nullptr);
  }

  static void clone(Allocator<countset_node> auto&& alloc,
                    countset_node* from,
                    countset_node*& to,
                    countset_node* new_parent) {
    if (from == nullptr) {
      return;
    }
    to = make(alloc, from);
    to->parent = new_parent;
    clone(alloc, from->left, to->left, to);
    clone(alloc, from->right, to->right, to);
  }

  template<class... Args>
  static countset_node* make(Allocator<countset_node> auto&& alloc,
                             Args&&... args) {
    countset_node* node = alloc.allocate(1);
#ifdef __EXCEPTIONS
    try {
#endif
      new (node) countset_node(std::forward<Args>(args)...);
#ifdef __EXCEPTIONS
    } catch (...) {
      alloc.deallocate(node, 1);
      throw;
    }
#endif
    return node;
  }

  template<class A>
  static countset_node* clone_tree(countset_node* node, A& alloc) {
    if (node == nullptr) {
      return nullptr;
    }
    using alloc_t =
        typename std::allocator_traits<A>::template rebind_alloc<countset_node>;
    alloc_t n_alloc(alloc);
    auto new_root = make(n_alloc, node);
#ifdef __EXCEPTIONS
    try {
#endif
      clone(n_alloc, node->left, new_root->left, new_root);
      clone(n_alloc, node->right, new_root->right, new_root);
#ifdef __EXCEPTIONS
    } catch (...) {
      clear_node(new_root, n_alloc);
      throw;
    }
#endif
    return new_root;
  }

  static void clear_node(countset_node* node,
                         Allocator<countset_node> auto&& alloc) noexcept {
    if (node->left != nullptr) {
      clear_node(node->left, alloc);
    }
    if (node->right != nullptr) {
      clear_node(node->right, alloc);
    }
    node->~countset_node();
    alloc.deallocate(node, 1);
  }

  countset_node* parent = nullptr;
  countset_node* left = nullptr;
  countset_node* right = nullptr;
  std::size_t count = 1;
  std::size_t priority;
  T k;

  template<class U, Comparator<U> C, Allocator<U> A, RandomNumberGen R>
  friend struct countset;
};

template<class T, iterator_dir d>
struct countset_iterator {
  using iterator_category = std::bidirectional_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = const T;
  using reference = const T&;
  using pointer = const T*;

  explicit countset_iterator(const countset_node<T>* node) noexcept
          : node(node) {
  }
  countset_iterator() noexcept = default;
  countset_iterator(const countset_iterator&) noexcept = default;
  countset_iterator(countset_iterator&&) noexcept = default;
  countset_iterator& operator=(const countset_iterator&) noexcept = default;
  countset_iterator& operator=(countset_iterator&&) noexcept = default;
  ~countset_iterator() noexcept = default;

  [[nodiscard]] bool operator==(const countset_iterator&) const noexcept
      = default;

  reference operator*() const noexcept {
    return node->value();
  }

  pointer operator->() const noexcept {
    return std::addressof(node->value());
  }

  countset_iterator& operator++() noexcept {
    if constexpr (d == iterator_dir::fwd) {
      node = node->next();
    } else {
      node = node->prev();
    }
    return *this;
  }

  countset_iterator operator++(int) noexcept {
    countset_iterator tmp(*this);
    ++(*this);
    return tmp;
  }

  countset_iterator& operator--() noexcept {
    if constexpr (d == iterator_dir::fwd) {
      node = node->prev();
    } else {
      node = node->next();
    }
    return *this;
  }

  countset_iterator operator--(int) noexcept {
    countset_iterator tmp(*this);
    --(*this);
    return tmp;
  }

  [[nodiscard]] auto base() const noexcept {
    return countset_iterator<T, iterator_dir::fwd>(node);
  }

  [[nodiscard]] std::size_t index() const noexcept {
    return node->index();
  }

  [[nodiscard]] const countset_node<T>* get_node() const noexcept {
    return node;
  }

private:
  const countset_node<T>* node;
};

struct default_rng {
  std::size_t x = 123456789;
  std::size_t y = 362436069;
  std::size_t z = 521288629;

  [[nodiscard]] std::size_t operator()() noexcept {
    std::size_t t;
    x ^= x << std::size_t(16);
    x ^= x >> std::size_t(5);
    x ^= x << std::size_t(1);

    t = x;
    x = y;
    y = z;
    z = t ^ x ^ y;

    return z;
  }
};

template<class K, class T, class C>
concept key_for
    = (!std::is_convertible_v<K, countset_iterator<T, iterator_dir::fwd>>)
    && (!std::is_convertible_v<K, countset_iterator<T, iterator_dir::rev>>)
    && requires { typename C::is_transparent; };

template<class T,
         Comparator<T> C = std::less<>,
         Allocator<T> A = std::allocator<T>,
         RandomNumberGen R = default_rng>
struct countset {
  using key_type = T;
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using key_compare = C;
  using value_compare = C;
  using allocator_type = A;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = typename std::allocator_traits<A>::pointer;
  using const_pointer = typename std::allocator_traits<A>::const_pointer;
  using node_type = countset_node<T>;
  using node_allocator = typename std::allocator_traits<
      A>::template rebind_alloc<countset_node<T>>;
  using rng_type = R;
  using iterator = countset_iterator<T, iterator_dir::fwd>;
  using reverse_iterator = countset_iterator<T, iterator_dir::rev>;
  using const_iterator = iterator;
  using const_reverse_iterator = reverse_iterator;

  static_assert(std::is_nothrow_default_constructible_v<C>);
  static_assert(std::is_nothrow_move_constructible_v<C>);
  static_assert(std::is_nothrow_copy_constructible_v<C>);
  static_assert(std::is_nothrow_move_assignable_v<C>);
  static_assert(std::is_nothrow_swappable_v<C>);

  static_assert(std::is_nothrow_default_constructible_v<A>);
  static_assert(std::is_nothrow_move_constructible_v<A>);
  static_assert(std::is_nothrow_copy_constructible_v<A>);
  static_assert(std::is_nothrow_move_assignable_v<A>);
  static_assert(std::is_nothrow_swappable_v<A>);
  static_assert(std::is_nothrow_constructible_v<node_allocator, const A&>);

  static_assert(std::is_nothrow_default_constructible_v<R>);
  static_assert(std::is_nothrow_move_constructible_v<R>);
  static_assert(std::is_nothrow_copy_constructible_v<R>);
  static_assert(std::is_nothrow_move_assignable_v<R>);
  static_assert(std::is_nothrow_swappable_v<R>);

  /* implicit */ countset() noexcept = default;

  explicit countset(C cmp, A alloc = A(), R rng = R()) noexcept
          : cmp(std::move(cmp)), alloc(std::move(alloc)), rng(std::move(rng)) {
  }

  explicit countset(A alloc, R rng = R()) noexcept
          : alloc(std::move(alloc)), rng(std::move(rng)) {
  }

  explicit countset(R rng) noexcept: rng(std::move(rng)) {
  }

  /* implicit */ countset(const countset& other)
          : cmp(other.cmp), alloc(other.alloc), rng(other.rng),
            root(node_type::clone_tree(other.root, alloc)) {
  }

  explicit countset(const countset& other, C cmp)
          : cmp(std::move(cmp)), alloc(other.alloc), rng(other.rng),
            root(node_type::clone_tree(other.root, alloc)) {
  }

  explicit countset(const countset& other, C cmp, A alloc)
          : cmp(std::move(cmp)), alloc(std::move(alloc)), rng(other.rng),
            root(node_type::clone_tree(other.root, this->alloc)) {
  }

  explicit countset(const countset& other, C cmp, A alloc, R rng)
          : cmp(std::move(cmp)), alloc(std::move(alloc)), rng(std::move(rng)),
            root(node_type::clone_tree(other.root, this->alloc)) {
  }

  explicit countset(const countset& other, A alloc)
          : cmp(other.cmp), alloc(std::move(alloc)), rng(other.rng),
            root(node_type::clone_tree(other.root, this->alloc)) {
  }

  explicit countset(const countset& other, A alloc, R rng)
          : cmp(other.cmp), alloc(std::move(alloc)), rng(std::move(rng)),
            root(node_type::clone_tree(other.root, this->alloc)) {
  }

  explicit countset(const countset& other, R rng)
          : cmp(other.cmp), alloc(other.alloc), rng(std::move(rng)),
            root(node_type::clone_tree(other.root, this->alloc)) {
  }

  /* implicit */ countset(countset&& other) noexcept
          : cmp(std::move(other.cmp)), alloc(std::move(other.alloc)),
            rng(std::move(other.rng)), root(other.root) {
    other.root = nullptr;
  }

  explicit countset(countset&& other, C cmp) noexcept
          : cmp(std::move(cmp)), alloc(std::move(other.alloc)),
            rng(std::move(other.rng)), root(other.root) {
    other.root = nullptr;
  }

  explicit countset(countset&& other, C cmp, A alloc) noexcept
          : cmp(std::move(cmp)), alloc(std::move(alloc)),
            rng(std::move(other.rng)), root(other.root) {
    other.root = nullptr;
  }

  explicit countset(countset&& other, C cmp, A alloc, R rng) noexcept
          : cmp(std::move(cmp)), alloc(std::move(alloc)), rng(std::move(rng)),
            root(other.root) {
    other.root = nullptr;
  }

  explicit countset(countset&& other, A alloc) noexcept
          : cmp(std::move(other.cmp)), alloc(std::move(alloc)),
            rng(std::move(other.rng)), root(other.root) {
    other.root = nullptr;
  }

  explicit countset(countset&& other, A alloc, R rng) noexcept
          : cmp(std::move(other.cmp)), alloc(std::move(alloc)),
            rng(std::move(rng)), root(other.root) {
    other.root = nullptr;
  }

  explicit countset(countset&& other, R rng) noexcept
          : cmp(std::move(other.cmp)), alloc(std::move(other.alloc)),
            rng(std::move(rng)), root(other.root) {
    other.root = nullptr;
  }

  template<class InputIt>
  explicit countset(InputIt first,
                    InputIt last,
                    C cmp = C(),
                    A alloc = A(),
                    R rng = R())
          : cmp(std::move(cmp)), alloc(std::move(alloc)), rng(std::move(rng)) {
    ctor_insert(std::move(first), std::move(last));
  }

  template<class InputIt>
  explicit countset(InputIt first, InputIt last, A alloc, R rng = R())
          : alloc(std::move(alloc)), rng(std::move(rng)) {
    ctor_insert(std::move(first), std::move(last));
  }

  template<class InputIt>
  explicit countset(InputIt first, InputIt last, R rng): rng(std::move(rng)) {
    ctor_insert(std::move(first), std::move(last));
  }

  /* implicit */ countset(std::initializer_list<T> i_list) {
    ctor_insert(i_list.begin(), i_list.end());
  }

  explicit countset(std::initializer_list<T> i_list,
                    C cmp,
                    A alloc = A(),
                    R rng = R())
          : cmp(std::move(cmp)), alloc(std::move(alloc)), rng(std::move(rng)) {
    ctor_insert(i_list.begin(), i_list.end());
  }

  explicit countset(std::initializer_list<T> i_list, A alloc, R rng = R())
          : alloc(std::move(alloc)), rng(std::move(rng)) {
    ctor_insert(i_list.begin(), i_list.end());
  }

  explicit countset(std::initializer_list<T> i_list, R rng)
          : rng(std::move(rng)) {
    ctor_insert(i_list.begin(), i_list.end());
  }

  ~countset() {
    clear();
  }

  countset& operator=(const countset& other) {
    if (this != std::addressof(other)) {
      clear();
      cmp = other.cmp;
      alloc = other.alloc;
      root = node_type::clone_tree(other.root, alloc);
    }
    return *this;
  }

  countset& operator=(countset&& other) noexcept {
    if (this != std::addressof(other)) {
      clear();
      cmp = std::move(other.cmp);
      alloc = std::move(other.alloc);
      root = other.root;
      other.root = nullptr;
    }
    return *this;
  }

  void swap(countset& other) noexcept {
    if (this == std::addressof(other)) {
      return;
    }
    using std::swap;
    swap(cmp, other.cmp);
    swap(alloc, other.alloc);
    swap(root, other.root);
  }

  friend void swap(countset& lhs, countset& rhs) noexcept {
    lhs.swap(rhs);
  }

  [[nodiscard]] A get_allocator() const noexcept {
    return alloc;
  }

  [[nodiscard]] C key_comp() const noexcept {
    return cmp;
  }

  [[nodiscard]] C value_comp() const noexcept {
    return cmp;
  }

  [[nodiscard]] R get_rng() const noexcept {
    return rng;
  }

  [[nodiscard]] bool empty() const noexcept {
    return root == nullptr;
  }

  [[nodiscard]] std::size_t size() const noexcept {
    return root == nullptr ? 0 : root->count;
  }

  [[nodiscard]] std::size_t max_size() const noexcept {
    return std::numeric_limits<std::size_t>::max();
  }

  [[nodiscard]] auto begin() const noexcept {
    return root == nullptr ? iterator(nullptr) : iterator(root->leftmost());
  }
  [[nodiscard]] auto end() const noexcept {
    return iterator(nullptr);
  }

  [[nodiscard]] auto cbegin() const noexcept {
    return begin();
  }
  [[nodiscard]] auto cend() const noexcept {
    return end();
  }

  [[nodiscard]] auto rbegin() const noexcept {
    return root == nullptr ? reverse_iterator(nullptr)
                           : reverse_iterator(root->rightmost());
  }
  [[nodiscard]] auto rend() const noexcept {
    return reverse_iterator(nullptr);
  }

  [[nodiscard]] auto crbegin() const noexcept {
    return rbegin();
  }
  [[nodiscard]] auto crend() const noexcept {
    return rend();
  }

  std::pair<iterator, bool> insert(const T& key) {
    return insert_node(make_node(key));
  }

  iterator insert(iterator hint, const T& key) {
    return insert_node(hint, make_node(key));
  }

  std::pair<iterator, bool> insert(T&& key) {
    return insert_node(make_node(std::move(key)));
  }

  iterator insert(iterator hint, T&& key) {
    return insert_node(hint, make_node(std::move(key)));
  }

  void insert(std::initializer_list<T> i_list) {
    insert(i_list.begin(), i_list.end());
  }

  template<class InputIt>
  void insert(InputIt first, InputIt last) {
    while (first != last) {
      insert(*first);
      first++;
    }
  }

  template<class... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {
    return insert_node(make_node(std::forward<Args>(args)...));
  }

  template<class... Args>
  iterator emplace_hint(iterator hint, Args&&... args) {
    return insert_node(hint, make_node(std::forward<Args>(args)...));
  }

  iterator erase(iterator pos) noexcept {
    auto node = const_cast<node_type*>(pos.get_node());
    auto ret = ++pos;
    node_type::erase(root, node);
    node->~node_type();
    node_allocator(alloc).deallocate(node, 1);
    return ret;
  }

  iterator erase(iterator first, iterator last) noexcept {
    while (first != last) {
      first = erase(first);
    }
    return first;
  }

  std::size_t erase(const T& key) noexcept {
    const auto range = equal_range(key);
    const auto ret = count_between(range);
    erase(range.first, range.second);
    return ret;
  }

  std::size_t erase(const key_for<T, C> auto& key) noexcept {
    const auto range = equal_range(key);
    const auto ret = count_between(range);
    erase(range.first, range.second);
    return ret;
  }

  void clear() noexcept {
    if (root != nullptr) {
      node_type::clear_node(root, node_allocator(alloc));
    }
    root = nullptr;
  }

  std::size_t count(const T& key) const noexcept {
    return count_between(equal_range(key));
  }

  std::size_t count(const key_for<T, C> auto& key) const noexcept {
    return count_between(equal_range(key));
  }

  bool contains(const T& key) const noexcept {
    // TODO: Benchmark and optimize!
    return find(key) != end();
  }

  bool contains(const key_for<T, C> auto& key) const noexcept {
    // TODO: Benchmark and optimize!
    return find(key) != end();
  }

  iterator find(const T& key) const noexcept {
    if (root == nullptr) {
      return end();
    }
    return iterator(root->find(key, cmp));
  }

  iterator find(const key_for<T, C> auto& key) const noexcept {
    if (root == nullptr) {
      return end();
    }
    return iterator(root->find(key, cmp));
  }

  iterator lower_bound(const T& key) const noexcept {
    if (root == nullptr) {
      return end();
    }
    return iterator(
        root->lower_bound(key, [this](const auto& a, const auto& b) noexcept {
          return !cmp(b, a);
        }));
  }

  iterator lower_bound(const key_for<T, C> auto& key) const noexcept {
    if (root == nullptr) {
      return end();
    }
    return iterator(
        root->lower_bound(key, [this](const auto& a, const auto& b) noexcept {
          return !cmp(b, a);
        }));
  }

  iterator upper_bound(const T& key) const noexcept {
    if (root == nullptr) {
      return end();
    }
    return iterator(root->lower_bound(key, cmp));
  }

  iterator upper_bound(const key_for<T, C> auto& key) const noexcept {
    if (root == nullptr) {
      return end();
    }
    return iterator(root->lower_bound(key, cmp));
  }

  std::pair<iterator, iterator> equal_range(const T& key) const noexcept {
    // TODO: Benchmark and optimize!
    return {lower_bound(key), upper_bound(key)};
  }

  std::pair<iterator, iterator>
      equal_range(const key_for<T, C> auto& key) const noexcept {
    // TODO: Benchmark and optimize!
    return {lower_bound(key), upper_bound(key)};
  }

  // Non-standard API
  iterator nth(std::size_t n) const noexcept {
    if (n >= size()) {
      return end();
    }
    auto node = root;
    while (true) {
      const auto count_of_left = node_type::count_of(node->left);
      if (count_of_left < n) {
        n -= (count_of_left + 1);
        node = node->right;
      } else if (count_of_left > n) {
        node = node->left;
      } else {
        return iterator(node);
      }
    }
  }

  const node_type* get_root_node() const noexcept {
    return root;
  }

private:
  template<class It>
  void ctor_insert(It begin, It end) {
#ifdef __EXCEPTIONS
    try {
#endif
      insert(std::move(begin), std::move(end));
#ifdef __EXCEPTIONS
    } catch (...) {
      clear();
      throw;
    }
#endif
  }

  template<class... Args>
  node_type* make_node(Args&&... args) {
    return node_type::make(
        node_allocator(alloc), rng(), std::forward<Args>(args)...);
  }

  std::pair<iterator, bool> insert_node(node_type* node) noexcept {
    auto conflict_node = node_type::insert(root, node, cmp);
    if (conflict_node != nullptr) {
      node->~node_type();
      node_allocator(alloc).deallocate(node, 1);
      return {iterator(conflict_node), false};
    }
    return {iterator(node), true};
  }

  iterator insert_node(iterator /* next */, node_type* node) noexcept {
    // TODO: Benchmark and optimize!
    return insert_node(node).first;
  }

  std::size_t
      count_between(std::pair<iterator, iterator> range) const noexcept {
    if (range.first == end()) {
      return 0;
    }
    if (range.second == end()) {
      return size() - range.first.index();
    }
    return range.second.index() - range.first.index();
  }

  [[no_unique_address]] C cmp{};
  [[no_unique_address]] A alloc{};
  [[no_unique_address]] R rng{};
  [[no_unique_address]] node_type* root = nullptr;
};

template<class It,
         Comparator<iter_value_t<It>> C = std::less<>,
         Allocator<iter_value_t<It>> A = std::allocator<iter_value_t<It>>,
         RandomNumberGen R = default_rng>
countset(It, It, C = C(), A = A(), R = R())
    -> countset<iter_value_t<It>, C, A, R>;

template<class T,
         Comparator<T> C = std::less<>,
         Allocator<T> A = std::allocator<T>,
         RandomNumberGen R = default_rng>
countset(std::initializer_list<T>, C = C(), A = A(), R = R())
    -> countset<T, C, A, R>;

template<class It,
         Allocator<iter_value_t<It>> A,
         RandomNumberGen R = default_rng>
countset(It, It, A, R = R()) -> countset<iter_value_t<It>, std::less<>, A, R>;

template<class T, Allocator<T> A, RandomNumberGen R = default_rng>
countset(std::initializer_list<T>, A, R = R())
    -> countset<T, std::less<>, A, R>;

template<class It, RandomNumberGen R>
countset(It, It, R) -> countset<iter_value_t<It>,
                                std::less<>,
                                std::allocator<iter_value_t<It>>,
                                R>;

template<class T, RandomNumberGen R>
countset(std::initializer_list<T>, R)
    -> countset<T, std::less<>, std::allocator<T>, R>;

}  // namespace eeds
