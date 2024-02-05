#pragma once
template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end, size_t range):
                    begin_(begin),end_(end),range_(range){}
    Iterator GetBegin() const{
        return begin_;
    }
    Iterator GetEnd() const{
        return end_;
    }
    Iterator GetRange() const{
        return range_;
    }
private:
    Iterator begin_;
    Iterator end_;
    size_t range_;
};

template <typename Iterator>
std::ostream& operator<< (std::ostream& out, const IteratorRange<Iterator>& page){
    for(Iterator it = page.GetBegin(); it != page.GetEnd();++it){
        out << *it;
    }
    return out;
}

template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator begin, Iterator end, size_t page_size){
        int range = distance(begin, end);
        int iter_counter = page_size;
        if(page_size !=0 ){
            if(page_size >= range){
                this->pages_.push_back({begin, end, page_size});
            }
            else {
                if(page_size == 1){
                    for(int i = 0;i < range/page_size;++i){
                        if(i != 0){
                            advance(begin, 1);
                        }
                        std::advance(end,-(range - iter_counter));
                        pages_.push_back({begin,end,page_size});
                        std::advance(end,range - iter_counter);

                        iter_counter += 1;
                    }
                }
                else{
                    for(int i = 0;i < range/iter_counter;++i){
                        if(i != 0){
                            std::advance(begin, iter_counter - 2);
                        }
                        std::advance(end,-(range - iter_counter));
                        pages_.push_back({begin,end,page_size});
                        std::advance(end,range - iter_counter);

                        iter_counter += iter_counter;
                    }
                    if(range != page_size && (begin != (end-2))){
                        std::advance(begin, page_size);
                        pages_.push_back({begin, end, page_size});
                    }
                }
            }
        }
    }

    auto begin() const {
        return pages_.begin();
    }
    auto end() const {
        return pages_.end();
    }

private:
    std::vector<IteratorRange<Iterator>> pages_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}