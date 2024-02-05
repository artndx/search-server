#pragma once

#include "document.h"
#include "read_input_functions.h"
#include "string_processing.h"
#include "paginator.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <stdexcept>
#include <deque>
#include <stdint.h>
#include <numeric>

const int MAX_RESULT_DOCUMENT_COUNT = 5;


class SearchServer {
public:
    
    template <typename StringContainer>
    SearchServer(const StringContainer& stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
        for(const std::string& word : stop_words_){
            if(!IsValidWord(word)){
                throw std::invalid_argument("Invalid chars in stop words");
            }
        }
    }


    SearchServer(const std::string& stop_words_text)
        : SearchServer(SplitIntoWords(stop_words_text))  
    {
    }

    void AddDocument(int document_id, const std::string& document, DocumentStatus status,
                     const std::vector<int>& ratings);
    
    int GetDocumentId(int index);

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, 
                                        DocumentPredicate document_predicate) const;

    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status) const;

    std::vector<Document> FindTopDocuments(const std::string& raw_query) const;

    int GetDocumentCount() const;

    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    const std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::vector<int> added_documents_id_;
    
    bool static IsValidWord(const std::string& word);

    bool IsStopWord(const std::string& word) const;

    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

    int static ComputeAverageRating(const std::vector<int>& ratings);

    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(std::string text) const;

    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    Query ParseQuery(const std::string& text) const;
    
    bool QueryIsCorrect(const Query& query) const;

    // Existence required
    double ComputeWordInverseDocumentFreq(const std::string& word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query,
                                        DocumentPredicate document_predicate) const;
};

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, 
                                    DocumentPredicate document_predicate) const{
    const Query query = ParseQuery(raw_query);
    if (!QueryIsCorrect(query)) {
        throw std::invalid_argument("Invalid query");
    }
    std::vector<Document> matched_documents = FindAllDocuments(query, document_predicate);
    
    std::sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
            if (std::abs(lhs.relevance - rhs.relevance) < std::numeric_limits<double>::epsilon()) {
                    return lhs.rating > rhs.rating;
            }
            return lhs.relevance > rhs.relevance;
            
            });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    
    return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query,
                                    DocumentPredicate document_predicate) const {
    std::map<int, double> document_to_relevance;
    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
            const auto& document_data = documents_.at(document_id);
            if (document_predicate(document_id, document_data.status, document_data.rating)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }

    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }

    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance) {
        matched_documents.push_back(
            {document_id, relevance, documents_.at(document_id).rating});
    }
    return matched_documents;
}