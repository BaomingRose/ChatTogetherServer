#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>

/*
 * author: rose
 * date: 2021.2.20
 * comment: 线程安全的<username，cliaddr>
 */
class Online_Map {
private:
    std::unordered_map<std::string, sockaddr_in> on_map;
    pthread_rwlock_t rw_lock;
public:
    Online_Map() {
        pthread_rwlock_init(&rw_lock, NULL);
    }

    ~Online_Map() {
        pthread_rwlock_destroy(&rw_lock);
    }

    void insert(const std::string& s, const sockaddr_in& addr) {
        pthread_rwlock_wrlock(&rw_lock);
        on_map.emplace(s, addr);
        pthread_rwlock_unlock(&rw_lock);

        std::cout << "online new user:" << s << "上线" << std::endl;
    }

    void erase(const std::string& username) {
        pthread_rwlock_wrlock(&rw_lock);
        auto it = on_map.find(username);
        if (it != on_map.end()) {
            on_map.erase(it);
        }
        pthread_rwlock_unlock(&rw_lock);

        std::cout << "downline user:" << username << "下线" << std::endl;
    }

    std::vector<sockaddr_in> get_addrs() {
        std::vector<sockaddr_in> v;
        pthread_rwlock_rdlock(&rw_lock);
        for (const auto& p : on_map) {
            v.push_back(p.second);
        }
        pthread_rwlock_unlock(&rw_lock);
        return v;
    }

    std::vector<std::string> get_users() {
        std::vector<std::string> v;
        pthread_rwlock_rdlock(&rw_lock);
        for (const auto& p : on_map) {
            v.push_back(p.first);
        }
        pthread_rwlock_unlock(&rw_lock);
        return v;
    }
};

