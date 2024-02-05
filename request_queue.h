#pragma once
#include "search_server.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server): 
        search_server_(search_server),
        no_results_requests_(0),
        current_time_(0){
    }

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;
private:
    struct QueryResult {
        uint64_t timestamp;
        int results;
    };

    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    const SearchServer& search_server_;
    int no_results_requests_;
    uint64_t current_time_;

    void AddRequest(int results_num);
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        const std::vector<Document> result = search_server_.FindTopDocuments(raw_query, document_predicate);
        int results_num = static_cast<int>(result.size());
        AddRequest(results_num);
        return result;
    }