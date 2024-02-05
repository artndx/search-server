#include "request_queue.h"

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    const std::vector<Document> result = search_server_.FindTopDocuments(raw_query,status);
    int results_num = static_cast<int>(result.size());
    AddRequest(results_num);
    return result;
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    const std::vector<Document> result = search_server_.FindTopDocuments(raw_query);
    int results_num = static_cast<int>(result.size());
    AddRequest(results_num);
    return result;
}
int RequestQueue::GetNoResultRequests() const {
    return no_results_requests_;
}

void RequestQueue::AddRequest(int results_num){
    ++current_time_;
    while(!requests_.empty() && min_in_day_ <= current_time_ - requests_.front().timestamp){
        if (0 == requests_.front().results) {
            --no_results_requests_;
        }
    requests_.pop_front();
    }
    requests_.push_back({current_time_,results_num});
    if(results_num == 0){
        ++no_results_requests_;
    }
}
