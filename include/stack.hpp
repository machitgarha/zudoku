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
        T &&move_top()
        {
            T &&result = std::move(*this->c.rbegin().operator->());

            this->pop();
            return std::move(result);
        }
    };
}

#endif // ZUDOKU_STACK_HPP
