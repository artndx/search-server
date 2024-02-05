#include "search_server.h"


void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status,
                    const std::vector<int>& ratings) {
    if (document_id < 0) {
        throw std::invalid_argument("Negative document id");
    }
    if (std::count (added_documents_id_.begin(), added_documents_id_.end(), document_id) != 0) {
        throw std::invalid_argument("Adding existing documen id");
    }
    const std::vector<std::string> words = SearchServer::SplitIntoWordsNoStop(document);
    for (const std::string& word : words) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("Invalid chars in document");
        }
    }
    const double inv_word_count = 1.0 / static_cast<double>(words.size());
    for (const std::string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    added_documents_id_.push_back(document_id);
}

int SearchServer::GetDocumentId(int index) {
    if (index < 0 || index > added_documents_id_.size()) {
        throw std::out_of_range("Out of range of added documents");
    }
    return added_documents_id_[index];
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status) const {
    return FindTopDocuments(
        raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const{
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {
    const SearchServer::Query query = SearchServer::ParseQuery(raw_query);
    
    std::vector<std::string> matched_words;
    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    std::tuple<std::vector<std::string>, DocumentStatus> result{matched_words, documents_.at(document_id).status};
    return result;
}


bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) > 0;
}

bool  SearchServer::IsValidWord(const std::string& word){
    // A valid word must not contain special characters
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsStopWord(word)) {
            if(!IsValidWord(word)){
                throw std::invalid_argument("Invalid word");
            }
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings){
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = accumulate(ratings.begin(),ratings.end(),0);
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string text) const {
    bool is_minus = false;
    // Word shouldn't be empty
    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
        if(text.empty()){
            text = " ";
        }
    }
    return {text, is_minus, IsStopWord(text)};
}


SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
    Query query;
    for (const std::string& word : SplitIntoWords(text)) {
        const QueryWord query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                query.minus_words.insert(query_word.data);
            } else {
                query.plus_words.insert(query_word.data);
            }
        }
    }
    if (!QueryIsCorrect(query)) {
        throw std::invalid_argument("Invalid query");
    }
    return query;
}

bool SearchServer::QueryIsCorrect(const Query& query) const {
    for(const std::string& query_word : query.plus_words){
        if(!IsValidWord(query_word)){
            return false;
        }
        if(count(query_word.begin(),query_word.end(),'-')){
            if(query_word[0] == '-' || query_word[query_word.size()-1] == '-'){
                return false;
            }
        }
        if(query_word == " "){
            return false;
        }
    }
    for(const std::string& query_word : query.minus_words){
        if(!IsValidWord(query_word)){
            return false;
        }
        if(count(query_word.begin(),query_word.end(),'-')){
            if(query_word[0] == '-' || query_word[query_word.size()-1] == '-'){
                return false;
            }
        }
        if(query_word == " "){
            return false;
        }
    }
    return true;
}

// Existence required
double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}


