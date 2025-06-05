
/******************************************************************************

© Intel Corporation.

This software and the related documents are Intel copyrighted materials,
and your use of them is governed by the express license under which they
were provided to you ("License"). Unless the License provides otherwise,
you may not use, modify, copy, publish, distribute, disclose or transmit
this software or the related documents without Intel's prior written
permission.

 This software and the related documents are provided as is, with no express
or implied warranties, other than those that are expressly stated in the
License.

******************************************************************************/

#pragma once

#include <algorithm>
#include <cassert>
#include <utility>

/*
REFERENCE :
    https://code.google.com/archive/p/intervaltree/
    https://code.google.com/archive/p/self-balancing-avl-tree/
*/

namespace gvk {

/**
Interval<> for use as a key for IntervalTree<>
@param <EndPointType> The type of the Interval<> end points
*/
template<typename EndPointType>
using Interval = std::pair<EndPointType, EndPointType>;

/**
Gets a value indicating whether or not two Intervals<> overlap
@param <EndPointType> The type of the Interval<> end points
@param [in] lhs The left hand side Interval<> to compare
@param [in] rhs The right hand side Interval<> to compare
@return Whether or not the given Interval<>s overlap
*/
template<typename EndPointType>
bool overlap(const Interval<EndPointType>& lhs, const Interval<EndPointType>& rhs)
{
    return lhs.first <= rhs.second && lhs.second >= rhs.first;
}

/**
@brief A sorted associative container of Intervals<> that provides enumeration over all Intervals<> that overlap a given interval
@param <EndPointType> The type of Interval<> end point
@param <ValueType> The type of value to associate with each Interval<>
*/
template<typename EndPointType, typename ValueType>
class IntervalTree final
{
public:
    /**
    @param Constructs an instance of IntervalTree<>
    */
    IntervalTree() = default;

    /**
    Moves an instance of IntervalTree<>
    * @param [in] other The IntervalTree<> to move from
    */
    IntervalTree(IntervalTree<EndPointType, ValueType>&& other)
    {
        *this = std::move(other);
    }

    /**
    Moves an instance of IntervalTree<>
    @param [in] other The IntervalTree<> to move from
    @return This IntervalTree<> after being moved to
    */
    IntervalTree<EndPointType, ValueType>& operator=(IntervalTree<EndPointType, ValueType>&& other)
    {
        mpRoot = std::move(other.mpRoot);
        other.mpRoot = nullptr;
        return *this;
    }

    /**
    Destroys this instance of IntervalTree<>
    */
    ~IntervalTree()
    {
        delete mpRoot;
    }

    /**
    Gets a reference to the value associated with a given EndPointType, performing an insertion if the given EndPointType does not already exist
    @param [in] point The EndPointType of the value to find
    @return A reference to the value associated with the given EndPointType
    */
    ValueType& operator[](const EndPointType& point)
    {
        return operator[]({ point, point });
    }

    /**
    Gets a reference to the value associated with a given Interval<>, performing an insertion if the given Interval<> does not already exist
    @param [in] interval The Interval<> of the value to find
    @return A reference to the value associated with the given Interval<>
    */
    ValueType& operator[](const Interval<EndPointType>& interval)
    {
        bool inserted = false;
        Node* pItr = nullptr;
        mpRoot = Node::insert(mpRoot, interval, inserted, &pItr);
        Node::compute_max(mpRoot);
        assert(pItr && "pItr must not be null after call to insert(); gvk maintenance required");
        return pItr->mValue;
    }

    /**
    Erases a given Interval<> from the IntervalTree<>
    @param [in] interval The Interval<> to erase
    */
    void erase(const Interval<EndPointType>& interval)
    {
        if (mpRoot) {
            bool erased = false;
            mpRoot = Node::erase(mpRoot, interval, erased);
            Node::compute_max(mpRoot);
        }
    }

    /**
    Enumerates all Interval<> objects that overlap a given Interval<>, calling a given enumeration function for each
    @param <EnumerationFunctionType> The type of function to call for each overlapping Interval<>
    @param [in] point The EndPointType to find overlapping Interval<> objects for
    @param [in] enumerationFunction The function to call for each overlapping Interval<>
        @note This function must have a signature compatible with `void(Interval<EndPointType>, ValueType)`
    */
    template<typename EnumerationFunctionType>
    void enumerate(EndPointType point, EnumerationFunctionType enumerationFunction) const
    {
        enumerate({ point, point }, enumerationFunction);
    }

    /**
    Enumerates all Interval<> objects that overlap a given Interval<>, calling a given enumeration function for each
    @param <EnumerationFunctionType> The type of function to call for each overlapping Interval<>
    @param [in] interval The Interval<> to find overlapping Interval<> objects for
    @param [in] enumerationFunction The function to call for each overlapping Interval<>
        @note This function must have a signature compatible with `void(Interval<EndPointType>, ValueType)`
    */
    template<typename EnumerationFunctionType>
    void enumerate(const Interval<EndPointType>& interval, EnumerationFunctionType enumerationFunction) const
    {
        if (mpRoot) {
            Node::enumerate(mpRoot, interval, enumerationFunction);
        }
    }

private:
    /**
    Used to track Interval<> objects stored in an IntervalTree<>
    */
    class Node final
    {
    public:
        /**
        Constructs an instance of Node
        */
        Node() = default;

        /**
        Destroys this instance of Node
        */
        ~Node()
        {
            delete mpLeft;
            delete mpRight;
        }

        /**
        Recursivley enumerates all Interval<> objects that overlap a given Interval<> under a given Node, calling a given enumeration function for each
        @param <EnumerationFunctionType> The type of function to call for each overlapping Interval<>
        @param [in] pNode A pointer to the Node under which to search recursively for overlapping Interval<> objects
        @param [in] interval The Interval<> to find overlapping Interval<>s for
        @param [in] enumerationFunction The function to call for each overlapping Interval<>
            @note This function must have a signature compatible with `void(Interval<EndPointType>, ValueType)`
        */
        template<typename EnumerationFunctionType>
        static void enumerate(const Node* pNode, const Interval<EndPointType>& interval, EnumerationFunctionType enumerationFunction)
        {
            assert(pNode && "enumerate<>() must not be called with nullptr for node; gvk maintenance required");
            if (interval.second < pNode->mInterval.first) {
                if (pNode->mpLeft) {
                    enumerate(pNode->mpLeft, interval, enumerationFunction);
                }
                if (overlap(pNode->mInterval, interval)) {
                    enumerationFunction(pNode->mInterval, pNode->mValue);
                }
            } else if (interval.first > pNode->mMax) {
                if (overlap(pNode->mInterval, interval)) {
                    enumerationFunction(pNode->mInterval, pNode->mValue);
                }
                if (pNode->mpRight) {
                    enumerate(pNode->mpRight, interval, enumerationFunction);
                }
            } else {
                if (pNode->mpLeft) {
                    enumerate(pNode->mpLeft, interval, enumerationFunction);
                }
                if (overlap(pNode->mInterval, interval)) {
                    enumerationFunction(pNode->mInterval, pNode->mValue);
                }
                if (pNode->mpRight) {
                    enumerate(pNode->mpRight, interval, enumerationFunction);
                }
            }
        }

        /**
        Inserts or finds a given Interval<> under a given Node
        @param [in] pNode The Node to insert or find the given Interval<> under
        @param [in] interval The Interval<> to insert or find
        @param [in] inserted Whether or not a new Node has been inserted
        @param [out] ppItr A pointer to a pointer to the inserted or found Node
        */
        static Node* insert(Node* pNode, const Interval<EndPointType>& interval, bool& inserted, Node** ppItr)
        {
            assert(ppItr && "insert() must not be called with nullptr for ppItr; gvk maintenance required");
            if (!pNode) {
                pNode = new Node;
                pNode->mInterval = interval;
                pNode->mMax = interval.second;
                inserted = true;
                *ppItr = pNode;
            } else {
                Node* pNewNode = nullptr;
                if (interval < pNode->mInterval) {
                    pNewNode = insert(pNode->mpLeft, interval, inserted, ppItr);
                    if (pNewNode != pNode->mpLeft) {
                        pNode->mpLeft = pNewNode;
                    }
                    if (inserted) {
                        --pNode->mBalance;
                        if (pNode->mBalance == 0) {
                            inserted = false;
                        } else if (pNode->mBalance == -2) {
                            if (pNode->mpLeft->mBalance == 1) {
                                int leftRightBalance = pNode->mpLeft->mpRight->mBalance;
                                pNode->mpLeft = rotate_left(pNode->mpLeft);
                                pNode = rotate_right(pNode);
                                pNode->mBalance = 0;
                                pNode->mpLeft->mBalance = leftRightBalance == 1 ? -1 : 0;
                                pNode->mpRight->mBalance = leftRightBalance == -1 ? 1 : 0;
                            } else if (pNode->mpLeft->mBalance == -1) {
                                pNode = rotate_right(pNode);
                                pNode->mBalance = 0;
                                pNode->mpRight->mBalance = 0;
                            }
                            inserted = false;
                        }
                    }
                } else if (interval == pNode->mInterval) {
                    *ppItr = pNode;
                } else if (interval > pNode->mInterval) {
                    pNewNode = insert(pNode->mpRight, interval, inserted, ppItr);
                    if (pNewNode != pNode->mpRight) {
                        pNode->mpRight = pNewNode;
                    }
                    if (inserted) {
                        ++pNode->mBalance;
                        if (pNode->mBalance == 0) {
                            inserted = false;
                        } else if (pNode->mBalance == 2) {
                            if (pNode->mpRight->mBalance == -1) {
                                auto rightLeftBalance = pNode->mpRight->mpLeft->mBalance;
                                pNode->mpRight = rotate_right(pNode->mpRight);
                                pNode = rotate_left(pNode);
                                pNode->mBalance = 0;
                                pNode->mpLeft->mBalance = rightLeftBalance == 1 ? -1 : 0;
                                pNode->mpRight->mBalance = rightLeftBalance == -1 ? 1 : 0;
                            } else if (pNode->mpRight->mBalance == 1) {
                                pNode = rotate_left(pNode);
                                pNode->mBalance = 0;
                                pNode->mpLeft->mBalance = 0;
                            }
                            inserted = false;
                        }
                    }
                }
                compute_max(pNode);
            }
            assert(pNode && "insert() must not return nullptr; gvk maintenance required");
            return pNode;
        }

        /**
        Erases a given Interval<> from the subtree rooted at a given Node
        @param [in] pNode A pointer to the root of the subtree
        @param [in] interval The Interval<> to erase
        @param [out] erased Whether or not a Node was erased
        @return The new root of the subtree after the erase operation
        */
        static Node* erase(Node* pNode, const Interval<EndPointType>& interval, bool& erased)
        {
            if (pNode) {
                // TODO : Double check interval comparisons vs reference
                if (interval < pNode->mInterval) {
                    pNode->mpLeft = erase(pNode->mpLeft, interval, erased);
                    if (erased) {
                        ++pNode->mBalance;
                    }
                } else if (interval > pNode->mInterval) {
                    pNode->mpRight = erase(pNode->mpRight, interval, erased);
                    if (erased) {
                        --pNode->mBalance;
                    }
                } else {
                    // Found the node to delete
                    erased = true;
                    if (!pNode->mpLeft) {
                        Node* pRight = pNode->mpRight;
                        pNode->mpRight = nullptr;
                        delete pNode;
                        return pRight;
                    } else if (!pNode->mpRight) {
                        Node* pLeft = pNode->mpLeft;
                        pNode->mpLeft = nullptr;
                        delete pNode;
                        return pLeft;
                    } else {
                        // Node with two children: find the in-order successor
                        Node* pSuccessor = pNode->mpRight;
                        while (pSuccessor->mpLeft) {
                            pSuccessor = pSuccessor->mpLeft;
                        }
                        pNode->mInterval = pSuccessor->mInterval;
                        pNode->mValue = pSuccessor->mValue;
                        pNode->mpRight = erase(pNode->mpRight, pSuccessor->mInterval, erased);
                        if (erased) {
                            --pNode->mBalance;
                        }
                    }
                }

                // Rebalance the tree if necessary
                if (pNode->mBalance == -2) {
                    if (pNode->mpLeft->mBalance <= 0) {
                        pNode = rotate_right(pNode);
                    } else {
                        pNode->mpLeft = rotate_left(pNode->mpLeft);
                        pNode = rotate_right(pNode);
                    }
                } else if (pNode->mBalance == 2) {
                    if (pNode->mpRight->mBalance >= 0) {
                        pNode = rotate_left(pNode);
                    } else {
                        pNode->mpRight = rotate_right(pNode->mpRight);
                        pNode = rotate_left(pNode);
                    }
                }

                compute_max(pNode);
            }
            return pNode;
        }

        /**
        Computes the max end point value for the subtree at a given Node
        @param [in] pNode A pointer to the Node to compute the max end point value for
        */
        static void compute_max(Node* pNode)
        {
            assert(pNode && "compute_max() must not be called with nullptr for pNode; gvk maintenance required");
            if (!pNode->mpLeft && !pNode->mpRight) {
                pNode->mMax = pNode->mInterval.second;
            } else if (!pNode->mpLeft) {
                pNode->mMax = std::max(pNode->mMax, pNode->mpRight->mMax);
            } else if (!pNode->mpRight) {
                pNode->mMax = std::max(pNode->mMax, pNode->mpLeft->mMax);
            } else {
                pNode->mMax = std::max(std::max(pNode->mpLeft->mMax, pNode->mMax), pNode->mpRight->mMax);
            }
        }

        /**
        Rotates a given Node and its immediate right and right->left children to the left
        @param [in] pNode A pointer to the Node to rotate left
        */
        static Node* rotate_left(Node* pNode)
        {
            /*

                A (node)       B
                 \            /
                  B      ->  A
                 /            \
                C              C

            */
            assert(pNode && "rotate_left() must not be called with nullptr for pNode; gvk maintenance required");
            auto pRight = pNode->mpRight;
            pNode->mpRight = pRight->mpLeft;
            compute_max(pNode);
            pRight->mpLeft = pNode;
            compute_max(pRight);
            return pRight;
        }

        /**
        Rotates a given Node and its immediate left and left->right children to the right
        @param [in] pNode A pointer to the Node to rotate right
        */
        static Node* rotate_right(Node* pNode)
        {
            /*

                  A (node)         B
                 /                  \
                B          ->        A
                 \                  /
                  C                C

            */
            assert(pNode && "rotate_right() must not be called with nullptr for pNode; gvk maintenance required");
            auto pLeft = pNode->mpLeft;
            pNode->mpLeft = pLeft->mpRight;
            compute_max(pNode);
            pLeft->mpRight = pNode;
            compute_max(pLeft);
            return pLeft;
        }

        Interval<EndPointType> mInterval{ };
        ValueType mValue{ };
        EndPointType mMax{ };
        Node* mpLeft{ };
        Node* mpRight{ };
        int mBalance{ };

    private:
        Node(const Node&) = delete;
        Node& operator=(const Node&) = delete;
    };

    Node* mpRoot{ };

    IntervalTree(const IntervalTree<EndPointType, ValueType>&) = delete;
    IntervalTree& operator=(const IntervalTree<EndPointType, ValueType>&) = delete;
};

} // namespace gvk
