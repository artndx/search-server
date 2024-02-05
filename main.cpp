#include "request_queue.h"
using namespace std;
int main() {
    SearchServer search_server{vector<string>{"cat"}};
    search_server.AddDocument(1, std::string("curly cat curly tail"), DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, std::string("curly dog and fancy collar"), DocumentStatus::ACTUAL, {1, 2, 3});
    search_server.AddDocument(3, std::string("big cat fancy collar "), DocumentStatus::ACTUAL, {1, 2, 8});
    search_server.AddDocument(4, std::string("big dog sparrow Eugene"), DocumentStatus::ACTUAL, {1, 3, 2});
    search_server.AddDocument(5, std::string("big dog sparrow Vasiliy"), DocumentStatus::ACTUAL, {1, 1, 1});
    auto results = search_server.FindTopDocuments("-curly dog");
    for(const Document& result : results){
        std::cout << result;
        std::cout << '\n';
    } 
}
