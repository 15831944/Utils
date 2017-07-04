/***********************************************************************
模块名	: Utils
文件名	: BufferList.hpp
作者		: ZQJ
创建日期	: 2016/11/23
备注		: 简单链表类,减少资源的频繁申请和释放
**************************************************************************/

#pragma once
#include<list>

namespace Utils
{

    //ListNodeBase,定义双链表的前驱和后继节点
    struct ListNodeBase
    {
        ListNodeBase *m_pNext;
        ListNodeBase *m_pPrev;

    public:
        ListNodeBase() : m_pNext(NULL), m_pPrev(NULL) {}
    };

    //ListNode仅定义数据成员
    //ListNode继承ListNodeBase，目的在于节约空间，以及时数据操作和链表操作分离
    template<typename T>
    struct ListNode : public ListNodeBase
    {
        T m_Data;
    };

    //迭代器前置申明
    template<typename T> struct ListIterator;
    template<typename T> struct ListConstIterator;

    //ListBase,定义List的相关内存管理，及定义哨兵结点。
    //内存管理通过保存一个FreeList。申请结点时从FreeList中获取结点；释放结点时归还结点到FreeList中。
    template<typename T>
    class BuffListBase
    {
    public:
        BuffListBase() : m_pUserGuard(NULL),m_nFreeSize(0), m_pFreeHead(NULL)
        { 
            Initialize();
        }

        ~BuffListBase()
        {
            Finalize();
        }

    public:
        void Clear()
        {
            ListNodeBase *pCurNode = m_pFreeHead;
            while (NULL != pCurNode)
            {
                ListNodeBase *temp = pCurNode;
                pCurNode = pCurNode->m_pNext;
                delete temp;
            }

            m_nFreeSize = 0;
            m_pFreeHead = NULL;
        }

    protected:
        typedef ListNode<T> Node_t;

        //ListNodeBase m_nodeUser;
        //用户链表哨兵，用于衔接循环链表，及避免头指针判断。
        ListNodeBase *m_pUserGuard;
        
    protected:
        Node_t* CreateNode(const T& value)
        {
            Node_t* pNode = NULL;
            try
            {
                pNode = this->GetNode();
                if (NULL != pNode)
                {
                    pNode->m_Data = value;
                }
            }
            catch (...)
            {
                pNode = NULL;
            }

            return pNode;
        }

        void DestoryNode(Node_t* pNode)
        {
            this->PutNode(pNode);
        }

    protected:
        void Initialize()
        {
            if (NULL == m_pUserGuard)
            {
                m_pUserGuard = new ListNodeBase;
            }
            
            m_pUserGuard->m_pNext = m_pUserGuard;
            m_pUserGuard->m_pPrev = m_pUserGuard;
        }

        void Finalize()
        {
            Clear();

            if (NULL != m_pUserGuard)
            {
                delete m_pUserGuard;
                m_pUserGuard = NULL;
            }
        }

    private:
        enum { LIST_ITEM_MAX = 10000 };

        size_t m_nFreeSize;
        ListNodeBase *m_pFreeHead;

    private:
        //为提高性能，FreeList仅实现单链表；
        //且获取和归还结点均在头结点处处理，理论上时间复杂度为O(1)
        void PushFront(ListNodeBase *pNode)
        {
            pNode->m_pNext = m_pFreeHead;
            m_pFreeHead = pNode;
        }

        ListNodeBase* PullFront()
        {
            ListNodeBase *pNode = m_pFreeHead;
            m_pFreeHead = m_pFreeHead->m_pNext;
            return pNode;
        }

        Node_t* GetNode()
        {
            if (m_nFreeSize > 0)
            {
                --m_nFreeSize;
                return static_cast<Node_t *>(PullFront());
            }
            else
            {
                return new Node_t;
            }
        }

        void PutNode(ListNode<T>* pNode)
        {
            if (m_nFreeSize < LIST_ITEM_MAX)
            {
                PushFront(pNode);
                ++m_nFreeSize;
            }
            else
            {
                delete pNode;
            }
        }
    };

    template<typename T>
    class BuffList : protected BuffListBase<T>
    {
        typedef BuffListBase<T>               Base_t;

    public:
        typedef T                                      value_type;
        typedef T*                                     pointer;
        typedef const T*                               const_pointer;
        typedef T&                                     reference;
        typedef const T&                               const_reference;
        typedef ListIterator<T>                        iterator;
        typedef ListConstIterator<T>                   const_iterator;
        typedef std::reverse_iterator<const_iterator>  const_reverse_iterator;
        typedef std::reverse_iterator<iterator>        reverse_iterator;
        typedef size_t                                 size_type;
        typedef ptrdiff_t                              difference_type;

    public:
        BuffList(){}
        explicit BuffList(size_type size, const value_type& value = value_type())
        {
            this->Insert(Begin(), size, value);
        }

        BuffList(const BuffList& rhs) :BuffListBase()
        {
            this->Insert(Begin(), rhs.Begin(), rhs.End());
        }

        template<typename InputIterator>
        BuffList(InputIterator first, InputIterator last) : BuffListBase()
        {
            this->Insert(Begin(), first, last);
        }

        ~BuffList()
        {
            Clear();
        }

        BuffList& operator=(const BuffList& rhs)
        {
            if (rhs != *this)
            {
                iterator iterBeg = Begin();
                iterator iterEnd = End();
                const_iterator iterRhsBeg = rhs.Begin();
                const_iterator iterRhsEnd = rhs.End();

                for (; iterBeg != iterEnd && iterRhsBeg != iterRhsEnd; ++iterBeg, ++iterRhsBeg)
                {
                    *iterBeg = *iterRhsBeg;
                }

                if (iterRhsBeg == iterRhsEnd)
                {
                    Earse(iterBeg, iterEnd);
                }
                else
                {
                    Insert(iterEnd, iterRhsBeg, iterRhsEnd);
                }
            }

            return *this;
        }

    public:
        void Assign(size_type size, const value_type& value)
        {
            _FillAssign(size, value);
        }

        //template<typename InputIterator>
        //void Assign(InputIterator first, InputIterator last)
        //{
        //    _AssignDispatch<InputIterator>(first, last);
        //}

        iterator Begin()
        {
            return iterator(this->m_pUserGuard->m_pNext);
        }

        const_iterator Begin() const
        {
            return const_iterator(this->m_pUserGuard->m_pNext);
        }

        iterator End()
        {
            return iterator(this->m_pUserGuard);
        }

        const_iterator End() const
        {
            return const_iterator(this->m_pUserGuard);
        }

        reverse_iterator Rbegin()
        {
            return reverse_iterator(End());
        }

        const_reverse_iterator Rbegin() const
        {
            return const_reverse_iterator(End());
        }

        reverse_iterator Rend()
        {
            return reverse_iterator(Begin());
        }

        const_reverse_iterator Rend() const
        {
            return const_reverse_iterator(Begin());
        }

        bool Empty() const
        {
            return this->m_pUserGuard->m_pNext == this->m_pUserGuard;
        }

        size_type Size() const
        {
            return std::distance(Begin(), End());
        }

        size_type Max_size() const
        {
            return size_type(-1);
        }

        void Resize(size_type size, value_type value = value_type())
        {
            size_type len = 0;
            iterator iter = Begin();
            for (; iter != End() && len < size; ++iter, ++len)
            {
                ;//empty operater
            }

            if (len == size)
            {
                Erase(iter, End());
            }
            else
            {
                Insert(End(), size - len, value);
            }
        }

        reference Front()
        {
            return *Begin();
        }

        const_reference Front() const
        {
            return *Begin();
        }

        reference Back()
        {
            iterator temp = End();
            --temp;
            return *temp;
        }

        const_reference Back() const
        {
            const_iterator temp = End();
            --temp;
            return *temp;
        }

        void Push_fornt(const value_type& value)
        {
            this->_Insert(Begin(), value);
        }

        void Pop_front()
        {
            this->_Erase(Begin());
        }

        void Push_back(const value_type& value)
        {
            this->_Insert(End(), value);
        }

        void Pop_back()
        {
            this->_Erase(iterator(this->m_pUserGuard->m_pPrev));
        }

        iterator Insert(iterator pos, const value_type& value)
        {
            Node_t *temp = CreateNode(value);
            _Hook(temp, pos.m_pNode);
            return iterator(temp);
        }

        void Insert(iterator pos, size_type size, const value_type& value)
        {
            _FillInsert(pos, size, value);
        }

        template<typename InputIterator>
        void Insert(iterator pos, InputIterator first, InputIterator last)
        {
            _InsertDispatch(pos, first, last);
        }

        iterator Erase(iterator pos)
        {
            iterator retIter = iterator(pos.m_pNode->m_pNext);
            _Erase(pos);
            return retIter;
        }

        iterator Erase(iterator first, iterator last)
        {
            while (first != last)
            {
                first = Erase(first);
            }
            return last;
        }

        void Swap(BuffList& listTarget)
        {
            ListNodeBase *temp = this->m_pUserGuard;
            this->m_pUserGuard = listTarget.m_pUserGuard;
            listTarget.m_pUserGuard = temp;
        }

        void Clear()
        {
            iterator iter = Begin();
            for (; iter != End(); /*++iter*/)
            {
                iterator temp = iter;
                ++iter;
                DestoryNode(static_cast<Node_t*>(temp.m_pNode));
            }

            Base_t::Initialize();
        }

        void Splice(iterator pos, BuffList& listTarget)
        {
            if (!listTarget.Empty())
            {
                this->_Transfer(pos, listTarget.Begin(), listTarget.End());
            }
        }

        void Splice(iterator pos, BuffList& target, iterator targetPos)
        {
            iterator iter = targetPos;
            ++iter;
            if (pos == targetPos || pos == iter)
            {
                return;
            }

            this->_Transfer(pos, targetPos, iter);
        }

        void Splice(iterator pos, BuffList&target, iterator first, iterator last)
        {
            if (first != last)
            {
                this->_Transfer(pos, first, last);
            }
        }

        void Remove(const T& value)
        {
            iterator iterBeg = Begin();
            iterator iterEnd = End();

            while (iterBeg != iterEnd)
            {
                iterator iterNext = iterBeg;
                ++iterNext;
                if (value == *iterBeg)
                {
                    _Erase(iterBeg);
                }
                iterBeg = iterNext;
            }
        }

        template<typename Predicate>
        void Remove_if(Predicate pred)
        {
            iterator iterBeg = Begin();
            iterator iterEnd = End();
            while (iterBeg != iterEnd) 
            {
                iterator iterNext = iterBeg;
                ++iterNext;
                if (pred(*iterBeg))
                {
                    Erase(iterBeg);
                }
                iterBeg = iterNext;
            }
        }

        void Unique()
        {
            iterator iterBeg = Begin();
            iterator iterEnd = End();
            if (iterBeg == iterEnd)
            {
                return;
            }

            iterator iterNext = iterBeg;
            while (++iterNext != iterEnd)
            {
                if (*iterBeg == *iterNext)
                {
                    _Erase(iterNext);
                }
                else
                {
                    iterBeg = iterNext;
                }

                iterNext = iterBeg;
            }
        }

        template<typename BinaryPredicate>
        void Unique(BinaryPredicate binPred)
        {
            iterator iterBeg = Begin();
            iterator iterEnd = End();
            if (iterBeg == iterEnd)
            {
                return;
            }

            iterator iterNext = iterBeg;
            while (++iterNext != iterEnd)
            {
                if (binPred(*iterBeg,*iterNext))
                {
                    _Erase(iterNext);
                }
                else
                {
                    iterBeg = iterNext;
                }

                iterNext = iterBeg;
            }
        }

        void Merge(BuffList& target)
        {

            iterator iterBeg = Begin();
            iterator iterEnd = End();
            iterator iterTargetBeg = target.Begin();
            iterator iterTargetEnd = target.End();

            while (iterBeg != iterEnd && iterTargetBeg != iterTargetEnd)
            {
                if (*iterTargetBeg < *iterBeg)
                {
                    iterator iterNext = iterTargetBeg;
                    _Transfer(iterBeg, iterTargetBeg, ++iterNext);
                    iterTargetBeg = iterNext;
                }
                else
                {
                    ++iterBeg;
                }
            }

            if (iterTargetBeg != iterTargetEnd)
            {
                _Transfer(iterEnd, iterTargetBeg, iterTargetEnd);
            }
        }

        template<typename StrictWeakOrdering>
        void Merge(BuffList& target, StrictWeakOrdering comp)
        {
            iterator iterBeg = Begin();
            iterator iterEnd = End();
            iterator iterTargetBeg = target.Begin();
            iterator iterTargetEnd = target.End();

            while (iterBeg != iterEnd && iterTargetBeg != iterTargetEnd)
            {
                if (comp(*iterTargetBeg,*iterBeg))
                {
                    iterator iterNext = iterTargetBeg;
                    _Transfer(iterBeg, iterTargetBeg, ++iterNext);
                    iterTargetBeg = iterNext;
                }
                else
                {
                    ++iterBeg;
                }
            }

            if (iterTargetBeg != iterTargetEnd)
            {
                _Transfer(iterEnd, iterTargetBeg, iterTargetEnd);
            }
        }

        void Reverse()
        {
            if (this->m_pUserGuard->m_pNext == this->m_pUserGuard 
                || this->m_pUserGuard->m_pNext->m_pNext == this->m_pUserGuard)
            {
                return;
            }

            ListNodeBase *pCurNode = this->m_pUserGuard;
            do
            {
                ListNodeBase *temp = pCurNode->m_pNext;
                pCurNode->m_pNext = pCurNode->m_pPrev;
                pCurNode->m_pPrev = temp;

                pCurNode = pCurNode->m_pNext;
            } while (pCurNode != this->m_pUserGuard);
        }

        void Sort()
        {
            if (m_pUserGuard->m_pNext == m_pUserGuard || m_pUserGuard->m_pNext->m_pNext == m_pUserGuard)
            {
                return;
            }

            BuffList listCarry;
            BuffList listCounter[64];
            int nFill = 0;

            while (!Empty())
            {
                listCarry.Splice(listCarry.Begin(), *this, Begin());
                int i = 0;
                while (i < nFill && !listCounter[i].Empty())
                {
                    listCounter[i].Merge(listCarry);
                    listCarry.Swap(listCounter[i++]);
                }

                listCarry.Swap(listCounter[i]);
                if (i == nFill)
                {
                    ++nFill;
                }
            }

            for (int i = 1; i < nFill; ++i)
            {
                listCounter[i].Merge(listCounter[i - 1]);
            }

            Swap(listCounter[nFill - 1]);
        }

        template<typename StrictWeakOrdering>
        void Sort(StrictWeakOrdering comp)
        {
            if (m_pUserGuard->m_pNext == m_pUserGuard || m_pUserGuard->m_pNext->m_pNext == m_pUserGuard)
            {
                return;
            }

            BuffList listCarry;
            BuffList listCounter[64];
            int nFill = 0;

            while (!Empty())
            {
                listCarry.Splice(listCarry.Begin(), *this, Begin());
                int i = 0;
                while (i < nFill && !listCounter[i].Empty())
                {
                    listCounter[i].Merge(listCarry, comp);
                    listCarry.Swap(listCounter[i++]);
                }

                listCarry.Swap(listCounter[i]);
                if (i == nFill)
                {
                    ++nFill;
                }
            }

            for (int i = 1; i < nFill; ++i)
            {
                listCounter[i].Merge(listCounter[i - 1], comp);
            }

            Swap(listCounter[nFill - 1]);
        }

        void Show()
        {
            printf("show list: ");
            iterator iter = Begin();
            for (; iter != End(); ++iter)
            {
                printf("%d ", *iter);
            }
            printf("\n");
        }

    protected:
        typedef ListNode<T> Node_t;

    protected:
        template<typename InputIterator>
        void _AssignDispatch(InputIterator first, InputIterator last)
        {
            iterator iterBeg = Begin();
            iterator iterEnd = End();

            for (; iterBeg != iterEnd && first != last; ++iterBeg, ++first)
            {
                *iterBeg = *first;
            }

            if (first == last)
            {
                Erase(iterBeg, iterEnd);
            }
            else
            {
                Insert(iterEnd, first, last);
            }
        }

        void _FillAssign(size_type nSize, const value_type& value)
        {
            iterator iter = Begin();
            for (; iter != End() && nSize > 0; ++iter, --nSize)
            {
                *iter = value;
            }

            if (nSize > 0)
            {
                Insert(End(), nSize, value);
            }
            else
            {
                Erase(iter, End());
            }
        }

        template<typename InputIterator>
        void _InsertDispatch(iterator pos, InputIterator first, InputIterator last)
        {
            for (; first != last; ++first)
            {
                _Insert(pos, *first);
            }
        }

        void _FillInsert(iterator pos, size_type nSize, const value_type& value)
        {
            for (; nSize > 0; --nSize)
            {
                _Insert(pos, value);
            }
        }

        void _Transfer(iterator pos, iterator first, iterator last)
        {
            if (pos != last)
            {
                last.m_pNode->m_pPrev->m_pNext  = pos.m_pNode;
                first.m_pNode->m_pPrev->m_pNext = last.m_pNode;
                pos.m_pNode->m_pPrev->m_pNext   = first.m_pNode;

                ListNodeBase *temp     = pos.m_pNode->m_pPrev;
                pos.m_pNode->m_pPrev   = last.m_pNode->m_pPrev;
                last.m_pNode->m_pPrev  = first.m_pNode->m_pPrev;
                first.m_pNode->m_pPrev = temp;
            }
        }

        void _Insert(iterator pos, const value_type& value)
        {
            Node_t* temp = CreateNode(value);
            _Hook(temp, pos.m_pNode);
        }

        //当前结点位置添加结点
        void _Hook(ListNodeBase * const pPosNode,ListNodeBase *const pNode)
        {
            pPosNode->m_pNext = pNode;
            pPosNode->m_pPrev = pNode->m_pPrev;
            pNode->m_pPrev->m_pNext = pPosNode;
            pNode->m_pPrev = pPosNode;
        }

        //移除当前结点
        void _Unhook(ListNodeBase * const pPosNode)
        {
            pPosNode->m_pPrev->m_pNext = pPosNode->m_pNext;
            pPosNode->m_pNext->m_pPrev = pPosNode->m_pPrev;
        }

        void _Erase(iterator pos)
        {
            _Unhook(pos.m_pNode);
            Node_t* temp = static_cast<Node_t*>(pos.m_pNode);
            DestoryNode(temp);
        }
    };

    template<typename T>
    struct ListIterator
    {
        //类型定义
        typedef ListIterator<T>   Self_t;
        typedef ListNode<T>       Node_t;

        typedef ptrdiff_t                        difference_type;
        typedef std::bidirectional_iterator_tag  iterator_category;
        typedef T                                value_type;
        typedef T*                               pointer;
        typedef T&                               reference;

    public:
        //数据成员
        ListNodeBase *m_pNode;

    public:
        //构造函数
        ListIterator() : m_pNode(NULL){}
        explicit ListIterator(ListNodeBase *pNode) : m_pNode(pNode){}

    public:
        //操作符重载
        reference operator*() const
        {
            return static_cast<Node_t*>(m_pNode)->m_Data;
        }

        pointer operator->() const
        {
            return &static_cast<Node_t*>(m_pNode)->m_Data;
        }

        //prefix++
        Self_t& operator++()
        {
            m_pNode = m_pNode->m_pNext;
            return *this;
        }

        //postfix++
        Self_t operator++(int)
        {
            Self_t temp = *this;
            m_pNode = m_pNode->m_pNext;
            return temp;
        }

        //prefix--
        Self_t& operator--()
        {
            m_pNode = m_pNode->m_pPrev;
            return *this;
        }

        //postfix--
        Self_t operator--(int)
        {
            Self_t temp = *this;
            m_pNode = m_pNode->m_pPrev;
            return temp;
        }

        bool operator==(const Self_t& rhs) const
        {
            return m_pNode == rhs.m_pNode;
        }

        bool operator!=(const Self_t& rhs) const
        {
            return m_pNode != rhs.m_pNode;
        }
    };

    template<typename T>
    struct ListConstIterator
    {
    public:
        typedef ListConstIterator<T>               Self_t;
        typedef const ListNode<T>                  Node_t;
        typedef ListIterator<T>                    iterator;

        typedef ptrdiff_t                          difference_type;
        typedef std::bidirectional_iterator_tag    iterator_category;
        typedef T                                  value_type;
        typedef const T*                           pointer;
        typedef const T&                           reference;

    public:
        const ListNodeBase *m_pNode;

    public:
        ListConstIterator() : m_pNode(NULL) {}
        explicit ListConstIterator(const ListNodeBase *pNode) : m_pNode(pNode) {}
        ListConstIterator(const iterator& rhs) : m_pNode(rhs.m_pNode) {}

    public:
        reference operator*() const
        {
            return static_cast<Node_t*>(m_pNode)->m_Data;
        }

        pointer operator->() const
        {
            return &static_cast<Node_t*>(m_pNode)->m_Data;
        }

        Self_t& operator++()
        {
            m_pNode = m_pNode->m_pNext;
            return *this;
        }

        Self_t operator++(int)
        {
            Node_t temp = *this;
            m_pNode = m_pNode->m_pNext;
            return temp;
        }

        Self_t& operator--()
        {
            m_pNode = m_pNode->m_pPrev;
            return *this;
        }

        Self_t operator--(int)
        {
            Node_t temp = *this;
            m_pNode = m_pNode->m_pPrev;
            return temp;
        }

        bool operator==(const Self_t& rhs) const
        {
            return m_pNode == rhs.m_pNode;
        }

        bool operator!=(const Self_t& rhs) const
        {
            return m_pNode != rhs.m_pNode;
        }
    };

    template<typename T>
    inline bool operator==(const ListIterator<T>& lhs, const ListConstIterator<T>& rhs)
    {
        return lhs.m_pNode == rhs.m_pNode;
    }

    template<typename T>
    inline bool operator!=(const ListIterator<T>& lhs, const ListConstIterator<T>& rhs)
    {
        return lhs.m_pNode != rhs.m_pNode;
    }



    void TestBuffList()
    {
        BuffList<int> list;
        printf("size:%lu, empty:%d, ", list.Size(), list.Empty());
        printf("Front:%lu, Back:%d\n", list.Front(), list.Back());

        for (int index = 0; index < 12; ++index)
        {
            list.Push_back(index);
            //list.Clear();
            list.Begin();
            list.End();
            printf("size:%lu, empty:%d, ", list.Size(), list.Empty());
            printf("Front:%lu, Back:%d\n", list.Front(), list.Back());
        }
        list.Show();
        list.Pop_front();
        list.Pop_back();
        list.Show();

        BuffList<int> list2(list);
        list2.Show();
        BuffList<int> list3 = list;
        list3.Show();

        BuffList<int>::iterator iter = list.Begin();
        for (; iter != list.End();/*++iter*/)
        {
            printf("iterator:%d\n", *iter);
            iter = list.Erase(iter);
        }

        BuffList<int>::const_iterator cIter = list.Begin();
        for (; cIter != list.End(); ++cIter)
        {
            printf("const_iterator:%d\n", *cIter);
            //cIter = list.Erase(cIter);
        }

        BuffList<int>::reverse_iterator rIter = list.Rbegin();
        for (; rIter != list.Rend(); ++rIter)
        {
            printf("reverse_iterator:%d\n", *rIter);
            //rIter = list.Erase(rIter);
        }
        list.Show();

        iter = list.Begin();
        for (int index = 0; index < 12; ++index)
        {
            list.Insert(iter, index);
        }

        printf("Reverse===>:");
        list.Reverse();
        list.Show();
        list2.Show();

        printf("Swap===>:");
        list.Swap(list2);
        list.Show();
        list2.Show();

        //printf("Merge===>:");
        //list.Merge(list2);
        //list.Show();
        //list2.Show();

        printf("Swap===>:");
        list.Swap(list2);
        list.Show();
        list2.Show();

        printf("Sort===>:");
        list.Sort();
        list.Show();

        list.Push_back(5);
        list.Push_back(5);
        list.Push_back(5);
        list.Push_back(6);
        list.Push_back(6);
        printf("raw data:\n");
        list.Show();

        list.Unique();
        printf("Unique:\n");
        list.Show();

        list.Remove(5);
        printf("remove(5):\n");
        list.Show();

        list.Resize(3, -1);
        printf("Resize:\n");
        list.Show();

        list.Resize(30, 99);
        printf("Resize:\n");
        list.Show();

        list.Clear();
        list.Clear();
        printf("Clear:\n");
        list.Show();

        list.Assign(5, -1);
        printf("Assign:\n");
        list.Show();

        //list.Assign(list2.Begin(), list2.End());
        //printf("Assign:\n");
        //list.Show();
    }

}

