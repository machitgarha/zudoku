#ifndef ZUDOKU_STACK_HPP
#define ZUDOKU_STACK_HPP

#include <stack>

namespace Zudoku
{
    template<typename T, typename Container = std::deque<T>>
    class stack: public std::stack<T, Container>
    {
    public:
        using std::stack<T, Container>::stack;

        /**
         * Moves the top element value in the stack and removes it.
         */
        T move_top()
        {
            T result = std::move(*this->c.rbegin().operator->());

            this->pop();

            /*
             * Doing std::move() explicitly prevents compiler from doing move elision, and
             * should cause the move constructor being called twice (i.e. plus the move
             * above).
             */
            return result;
        }
    };
}

#endif // ZUDOKU_STACK_HPP
