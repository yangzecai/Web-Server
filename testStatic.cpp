#include <iostream>
#include <vector>

#define GETINDEX()                                                             \
    {                                                                          \
        static int index = -1;                                                 \
        return index;                                                          \
    }

void changeIndex(int& index)
{
    static std::vector<int> vec;
    if(index == -1) {
        index = vec.size();
        vec.push_back(1);
    }
}

int main() {}