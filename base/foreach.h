#pragma  once
namespace common {
    template<typename T, typename Func>
    void for_each_all(T& c, const Func& func) {
        typename T::iterator iter = c.begin();
        while (iter != c.end()) {
            func(*iter);
            ++iter;
        }
    }
    // example:
    // std::set<pid_t> pidSets;
    // for_each_all(pidSets, std::bind(::kill, _1, SIGTERM));
    
    template<class ForwardIterator>
    void STLDeleteContainerPointers(ForwardIterator begin, ForwardIterator end)
    {
        while (begin != end) {
            ForwardIterator temp = begin;
            ++begin;
            delete *temp;
        }
    }

    template<class T>
    void STLDeleteElements(T* container) {
        if (!container)
            return;
        STLDeleteContainerPointers(container->begin(), container->end());
        container->clear();
    }

    template <class ForwardIterator>
    void STLDeleteContainerSecondPointers(ForwardIterator begin,
        ForwardIterator end) {
        while (begin != end) {
            ForwardIterator temp = begin;
            ++begin;
            delete temp->second;
        }
    }
}
// end of local file.