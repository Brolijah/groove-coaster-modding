#include "common.h"
#include <string>

/*template<class T>
void pop_front(std::vector<T>& v)
{
	if (v.size() > 0) {
		v.erase(v.begin());
	}
}*/

/*template<typename T>
void pop_front(std::vector<T>& v) {
    if (v.size() > 0) {
        std::vector<T> initVector;
        for (int i = 0; i < v.size(); i++) {
            if (i == 0) continue;
            initVector.push_back(v[i]);
        }

        v = std::move(initVector);
    }
}*/