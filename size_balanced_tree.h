
#pragma once

#include <algorithm>
#include <numeric>
#include "binary_search_tree.h"

namespace zzz
{
    //incomplete
    template<class interface_t>
    class size_balanced_tree : protected binary_search_tree<interface_t>
    {
    public:
        typedef binary_search_tree<interface_t> base_t;
        typedef typename base_t::key_t key_t;
        typedef typename base_t::node_t node_t;
        typedef typename base_t::value_node_t value_node_t;
        class iterator
        {
        public:
            typedef std::random_access_iterator_tag iterator_category;
            typedef node_t value_type;
            typedef ptrdiff_t difference_type;
            typedef size_t distance_type;
            typedef node_t *pointer;
            typedef node_t reference;
        public:
            iterator(node_t *node) : ptr_(node)
            {
            }
            iterator(iterator const &other) : ptr_(other.ptr_)
            {
            }
            iterator &operator += (difference_type diff)
            {
                ptr_ = size_balanced_tree::sbt_advance_(ptr_, diff);
                return *this;
            }
            iterator &operator -= (difference_type diff)
            {
                ptr_ = size_balanced_tree::sbt_advance_(ptr_, -diff);
                return *this;
            }
            iterator operator + (difference_type diff)
            {
                return iterator(size_balanced_tree::sbt_advance_(ptr_, diff));
            }
            iterator operator - (difference_type diff)
            {
                return iterator(size_balanced_tree::sbt_advance_(ptr_, -diff));
            }
            difference_type operator - (iterator const &other)
            {
                return static_cast<ptrdiff_t>(size_balanced_tree::sbt_rank_(ptr_)) - static_cast<ptrdiff_t>(size_balanced_tree::sbt_rank_(other.ptr_));
            }
            iterator &operator++()
            {
                ptr_ = size_balanced_tree::template bst_move_<true>(ptr_);
                return *this;
            }
            iterator &operator--()
            {
                ptr_ = size_balanced_tree::template bst_move_<false>(ptr_);
                return *this;
            }
            iterator operator++(int)
            {
                iterator save(*this);
                ++*this;
                return save;
            }
            iterator operator--(int)
            {
                iterator save(*this);
                --*this;
                return save;
            }
            value_node_t &operator *()
            {
                return *static_cast<value_node_t *>(ptr_);
            }
            value_node_t *operator->()
            {
                return static_cast<value_node_t *>(ptr_);
            }
            bool operator == (iterator const &other) const
            {
                return ptr_ == other.ptr_;
            }
            bool operator != (iterator const &other) const
            {
                return ptr_ != other.ptr_;
            }
        private:
            node_t *ptr_;
        };
        friend class iterator;

    public:
        size_balanced_tree() : base_t()
        {
            set_size_(base_t::nil_(), 0);
        }
        size_balanced_tree(size_balanced_tree const &other) = delete;
        size_balanced_tree &operator = (size_balanced_tree const &other) = delete;
        
        template<class F>
        void safe_destroy(F const &callback)
        {
            if (!empty())
            {
                base_t::template bst_safe_destroy_(base_t::get_root_(), callback);
            }
        }

        typedef std::pair<iterator, iterator> pair_ii_t;

        iterator insert(value_node_t *node)
        {
            sbt_insert_(node);
            return iterator(node);
        }
        template<class iterator_t>
        size_t insert(iterator_t begin, iterator_t end)
        {
            size_t insert_count = 0;
            for(; begin != end; ++begin)
            {
                if(insert(*begin).second)
                {
                    ++insert_count;
                }
            }
            return insert_count;
        }
        iterator find(key_t const &key)
        {
            node_t *where = bst_lower_bound_(key);
            return (base_t::is_nil_(where) || interface_t::predicate(key, base_t::get_key_(where))) ? iterator(base_t::nil_()) : iterator(where);
        }
        void erase(iterator where)
        {
            sbt_erase_(&*where);
        }
        void erase(value_node_t *node)
        {
            sbt_erase_(node);
        }
        size_t erase(key_t const &key)
        {
            size_t erase_count = 0;
            pair_ii_t range = equal_range(key);
            while(range.first != range.second)
            {
                erase(range.first++);
                ++erase_count;
            }
            return erase_count;
        }
        size_t count(key_t const &key)
        {
            pair_ii_t range = equal_range(key);
            return std::distance(range.first, range.second);
        }
        size_t count(key_t const &min, key_t const &max)
        {
            return sbt_upper_rank_(max) - sbt_lower_rank_(min);
        }
        pair_ii_t range(key_t const &min, key_t const &max)
        {
            return pair_ii_t(bst_lower_bound_(min), bst_upper_bound_(max));
        }
        pair_ii_t slice(ptrdiff_t begin = 0, ptrdiff_t end = std::numeric_limits<ptrdiff_t>::max())
        {
            ptrdiff_t size_s = size();
            if(begin < 0)
            {
                begin = std::max<ptrdiff_t>(size_s + begin, 0);
            }
            if(end < 0)
            {
                end = size_s + end + 1;
            }
            if(begin > end || begin >= size_s)
            {
                return pair_ii_t(size_balanced_tree::end(), size_balanced_tree::end());
            }
            if(end > size_s)
            {
                end = size_s;
            }
            return pair_ii_t(size_balanced_tree::begin() + begin, size_balanced_tree::end() - (size_s - end));
        }
        iterator lower_bound(key_t const &key)
        {
            return iterator(bst_lower_bound_(key));
        }
        iterator upper_bound(key_t const &key)
        {
            return iterator(bst_upper_bound_(key));
        }
        pair_ii_t equal_range(key_t const &key)
        {
            node_t *lower, *upper;
            bst_equal_range_(key, lower, upper);
            return pair_ii_t(lower, upper);
        }
        iterator begin()
        {
            return iterator(base_t::get_most_left_());
        }
        iterator end()
        {
            return iterator(base_t::nil_());
        }
        value_node_t *front()
        {
            return static_cast<value_node_t *>(base_t::get_most_left_());
        }
        value_node_t *back()
        {
            return static_cast<value_node_t *>(base_t::get_most_right_());
        }
        bool empty()
        {
            return base_t::is_nil_(base_t::get_root_());
        }
        void clear()
        {
            base_t::bst_clear_();
        }
        size_t size()
        {
            return get_size_(base_t::get_root_());
        }
        value_node_t *at(size_t index)
        {
            return static_cast<value_node_t *>(sbt_at_(base_t::get_root_(), index));
        }
        size_t lower_rank(key_t const &key)
        {
            return sbt_lower_bound_(key);
        }
        size_t upper_rank(key_t const &key)
        {
          return sbt_upper_rank_(key);
        }
        static size_t rank(value_node_t *node)
        {
            return sbt_rank_(node);
        }
        static size_t rank(iterator where)
        {
            return sbt_rank_(&*where);
        }
    protected:
        static size_t get_size_(node_t *node)
        {
            return interface_t::get_size(node);
        }

        static void set_size_(node_t *node, size_t size)
        {
            interface_t::set_size(node, size);
        }

        void sbt_refresh_size_(node_t *node)
        {
            set_size_(node, get_size_(base_t::get_left_(node)) + get_size_(base_t::get_right_(node)) + 1);
        }

        static node_t *sbt_at_(node_t *node, size_t index)
        {
            if(index >= get_size_(node))
            {
                return nullptr;
            }
            size_t rank = get_size_(base_t::get_left_(node));
            while(index != rank)
            {
                if(index < rank)
                {
                    node = base_t::get_left_(node);
                }
                else
                {
                    index -= rank + 1;
                    node = base_t::get_right_(node);
                }
                rank = get_size_(base_t::get_left_(node));
            }
            return node;
        }

        static node_t *sbt_advance_(node_t *node, ptrdiff_t step)
        {
            if(base_t::is_nil_(node))
            {
                if(step == 0)
                {
                    return node;
                }
                else if(step > 0)
                {
                    --step;
                    node = base_t::get_left_(node);
                }
                else
                {
                    ++step;
                    node = base_t::get_right_(node);
                }
                if(base_t::is_nil_(node))
                {
                    return node;
                }
            }
            size_t u_step;
            while(step != 0)
            {
                if(step > 0)
                {
                    u_step = step;
                    if(get_size_(base_t::get_right_(node)) >= u_step)
                    {
                        step -= get_size_(base_t::get_left_(base_t::get_right_(node))) + 1;
                        node = base_t::get_right_(node);
                        continue;
                    }
                }
                else
                {
                    u_step = -step;
                    if(get_size_(base_t::get_left_(node)) >= u_step)
                    {
                        step += get_size_(base_t::get_right_(base_t::get_left_(node))) + 1;
                        node = base_t::get_left_(node);
                        continue;
                    }
                }
                if(base_t::is_nil_(base_t::get_parent_(node)))
                {
                    return base_t::get_parent_(node);
                }
                else
                {
                    if(base_t::get_right_(base_t::get_parent_(node)) == node)
                    {
                        step += get_size_(base_t::get_left_(node)) + 1;
                        node = base_t::get_parent_(node);
                    }
                    else
                    {
                        step -= get_size_(base_t::get_right_(node)) + 1;
                        node = base_t::get_parent_(node);
                    }
                }
            }
            return node;
        }

        static size_t sbt_rank_(node_t *node)
        {
            if(base_t::is_nil_(node))
            {
                return get_size_(get_parent_(node));
            }
            size_t rank = get_size_(base_t::get_left_(node));
            node_t *parent = get_parent_(node);
            while(!base_t::is_nil_(parent))
            {
                if(node == base_t::get_right_(parent))
                {
                    rank += get_size_(base_t::get_left_(parent)) + 1;
                }
                node = parent;
                parent = get_parent_(node);
            }
            return rank;
        }

        size_t sbt_lower_rank_(key_t const &key)
        {
            node_t *node = base_t::get_root_();
            size_t rank = 0;
            while (!base_t::is_nil_(node))
            {
                if (interface_t::predicate(base_t::get_key_(node), key))
                {
                    rank += get_size_(base_t::get_left_(node)) + 1;
                    node = base_t::get_right_(node);
                }
                else
                {
                    node = base_t::get_left_(node);
                }
            }
            return rank;
        }

        size_t sbt_upper_rank_(key_t const &key)
        {
            node_t *node = base_t::get_root_();
            size_t rank = 0;
            while (!base_t::is_nil_(node))
            {
                if (interface_t::predicate(key, base_t::get_key_(node)))
                {
                    node = base_t::get_left_(node);
                }
                else
                {
                    rank += get_size_(base_t::get_left_(node)) + 1;
                    node = base_t::get_right_(node);
                }
            }
            return rank;
        }

        template<bool is_left>
        node_t *sbt_rotate_(node_t *node)
        {
            node_t *child = base_t::template get_child_<!is_left>(node), *parent = base_t::get_parent_(node);
            base_t::template set_child_<!is_left>(node, base_t::template get_child_<is_left>(child));
            if(!base_t::is_nil_(base_t::template get_child_<is_left>(child)))
            {
              base_t::set_parent_(base_t::template get_child_<is_left>(child), node);
            }
            base_t::set_parent_(child, parent);
            if(node == base_t::get_root_())
            {
                base_t::set_root_(child);
            }
            else if(node == base_t::template get_child_<is_left>(parent))
            {
                base_t::template set_child_<is_left>(parent, child);
            }
            else
            {
                base_t::template set_child_<!is_left>(parent, child);
            }
            base_t::template set_child_<is_left>(child, node);
            base_t::set_parent_(node, child);
            set_size_(child, get_size_(node));
            sbt_refresh_size_(node);
            return child;
        }

        void sbt_insert_(node_t *key)
        {
            set_size_(key, 1);
            if(base_t::is_nil_(base_t::get_root_()))
            {
                base_t::set_root_(base_t::bst_init_node_(base_t::nil_(), key));
                base_t::set_most_left_(base_t::get_root_());
                base_t::set_most_right_(base_t::get_root_());
                return;
            }
            node_t *node = base_t::get_root_(), *where = base_t::nil_();
            bool is_left = true;
            while(!base_t::is_nil_(node))
            {
                set_size_(node, get_size_(node) + 1);
                where = node;
                if((is_left = base_t::predicate(key, node)))
                {
                    node = base_t::get_left_(node);
                }
                else
                {
                    node = base_t::get_right_(node);
                }
            }
            if(is_left)
            {
                base_t::set_left_(where, node = base_t::bst_init_node_(where, key));
                if(where == base_t::get_most_left_())
                {
                    base_t::set_most_left_(node);
                }
            }
            else
            {
                base_t::set_right_(where, node = base_t::bst_init_node_(where, key));
                if(where == base_t::get_most_right_())
                {
                    base_t::set_most_right_(node);
                }
            }
            while(!base_t::is_nil_(where))
            {
                if(node == base_t::get_left_(where))
                {
                    where = sbt_maintain_<true>(where);
                }
                else
                {
                    where = sbt_maintain_<false>(where);
                }
                node = where;
                where = base_t::get_parent_(where);
            }
        }

        template<bool is_left>
        node_t *sbt_maintain_(node_t *node)
        {
            if(base_t::is_nil_(base_t::template get_child_<is_left>(node)))
            {
                return node;
            }
            if(get_size_(base_t::template get_child_<is_left>(base_t::template get_child_<is_left>(node))) > get_size_(base_t::template get_child_<!is_left>(node)))
            {
                node = sbt_rotate_<!is_left>(node);
            }
            else
            {
                if(get_size_(base_t::template get_child_<!is_left>(base_t::template get_child_<is_left>(node))) > get_size_(base_t::template get_child_<!is_left>(node)))
                {
                    sbt_rotate_<is_left>(base_t::template get_child_<is_left>(node));
                    node = sbt_rotate_<!is_left>(node);
                }
                else
                {
                    return node;
                };
            };
            if(!base_t::is_nil_(base_t::template get_child_<true>(node)))
            {
                sbt_maintain_<true>(base_t::template get_child_<true>(node));
            }
            if(!base_t::is_nil_(base_t::template get_child_<false>(node)))
            {
                sbt_maintain_<false>(base_t::template get_child_<false>(node));
            }
            node = sbt_maintain_<true>(node);
            node = sbt_maintain_<false>(node);
            return node;
        }

        void sbt_erase_(node_t *node)
        {
            node_t *erase_node = node;
            node_t *fix_node = node;
            node_t *fix_node_parent;
            while(!base_t::is_nil_((fix_node = base_t::get_parent_(fix_node))))
            {
                set_size_(fix_node, get_size_(fix_node) - 1);
            }

            if(base_t::is_nil_(base_t::get_left_(node)))
            {
                fix_node = base_t::get_right_(node);
            }
            else if(base_t::is_nil_(base_t::get_right_(node)))
            {
                fix_node = base_t::get_left_(node);
            }
            else
            {
                if(get_size_(base_t::get_left_(node)) > get_size_(base_t::get_right_(node)))
                {
                    sbt_erase_on_<true>(node);
                }
                else
                {
                    sbt_erase_on_<false>(node);
                }
                return;
            }
            fix_node_parent = base_t::get_parent_(erase_node);
            if(!base_t::is_nil_(fix_node))
            {
                base_t::set_parent_(fix_node, fix_node_parent);
            }
            if(base_t::get_root_() == erase_node)
            {
                base_t::set_root_(fix_node);
            }
            else if(base_t::get_left_(fix_node_parent) == erase_node)
            {
                base_t::set_left_(fix_node_parent, fix_node);
            }
            else
            {
                base_t::set_right_(fix_node_parent, fix_node);
            }
            if(base_t::get_most_left_() == erase_node)
            {
                base_t::set_most_left_(base_t::is_nil_(fix_node) ? fix_node_parent : base_t::template bst_most_<true>(fix_node));
            }
            if(base_t::get_most_right_() == erase_node)
            {
                base_t::set_most_right_(base_t::is_nil_(fix_node) ? fix_node_parent : base_t::template bst_most_<false>(fix_node));
            }
        }

        template<bool is_left>
        void sbt_erase_on_(node_t *node)
        {
            node_t *erase_node = node;
            node_t *fix_node;
            node_t *fix_node_parent;
            node = base_t::template bst_move_<!is_left>(node);
            fix_node = base_t::template get_child_<is_left>(node);
            fix_node_parent = node;
            while((fix_node_parent = base_t::get_parent_(fix_node_parent)) != erase_node)
            {
                set_size_(fix_node_parent, get_size_(fix_node_parent) - 1);
            }
            base_t::set_parent_(base_t::template get_child_<!is_left>(erase_node), node);
            base_t::template set_child_<!is_left>(node, base_t::template get_child_<!is_left>(erase_node));
            if(node == base_t::template get_child_<is_left>(erase_node))
            {
                fix_node_parent = node;
            }
            else
            {
                fix_node_parent = base_t::get_parent_(node);
                if(!base_t::is_nil_(fix_node))
                {
                    base_t::set_parent_(fix_node, fix_node_parent);
                }
                base_t::template set_child_<!is_left>(fix_node_parent, fix_node);
                base_t::template set_child_<is_left>(node, base_t::template get_child_<is_left>(erase_node));
                base_t::set_parent_(base_t::template get_child_<is_left>(erase_node), node);
            }
            if(base_t::get_root_() == erase_node)
            {
                base_t::set_root_(node);
            }
            else if(base_t::template get_child_<!is_left>(base_t::get_parent_(erase_node)) == erase_node)
            {
                base_t::template set_child_<!is_left>(base_t::get_parent_(erase_node), node);
            }
            else
            {
                base_t::template set_child_<is_left>(base_t::get_parent_(erase_node), node);
            }
            base_t::set_parent_(node, base_t::get_parent_(erase_node));
            sbt_refresh_size_(node);
        }

    };

}