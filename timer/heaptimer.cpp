#include "heaptimer.h"

bool HeapTimer::empty(){
    return heap_.empty();
}

//更新旧节点的超时时间
void HeapTimer::adjust(int id,int timeout){
    assert(!heap_.empty() && ref_.count(id) > 0);
    heap_[ref_[id]].expires = time(nullptr) + timeout;
    // printf("adjust: 设置sockfd为%d的超时时间为15s\n");
    siftdown_(ref_[id],heap_.size());
}

//添加新节点/已有节点，调整堆
void HeapTimer::add(TimerNode *timer){
    int &id = timer->id;
    assert(id >= 0);
    int i;
    if(ref_.count(id) == 0){
        i = heap_.size();
        ref_[id] = i;
        heap_.push_back(*timer);
        siftup_(i);
    }
    else{

        i = ref_[id];
        if(!siftdown_(i,heap_.size())){
            siftup_(i);
        }
    }
}

//清空堆和哈希表
void HeapTimer::clear(){
    ref_.clear();
    heap_.clear();
}

//清除超时节点
void HeapTimer::tick(){
    LOG_INFO("%s", "timer tick");
    Log::get_instance()->flush();

    if(heap_.empty()) return;

    while(!heap_.empty()){
        TimerNode node = heap_.front();
        //判断两个时间点之间的时间间隔是否大于0秒
        if(node.expires - time(nullptr) > 0) break;
        node.cb(node.user_data);
        pop();
    }
}

//弹出堆顶节点
void HeapTimer::pop(){
    assert(!heap_.empty());
    del_(heap_.front().id);
}

//获取下一个超时时间
int HeapTimer::GetNextTick(){
    tick();
    
    int res = -1;
    if(!heap_.empty()){
        res = heap_.front().expires - time(NULL);
        if(res < 0) res = 0;
    }
    return res;
}

//删除指定位置的节点
void HeapTimer::del_(int id){
    int index = ref_[id];
    assert(!heap_.empty() && index >= 0 && index < heap_.size());

    int i = index;
    int n = heap_.size()-1;

    //将要删除的节点换到队尾，调整堆
    if(i < n){
        SwapNode_(i,n);
        if(!siftdown_(i,n)){
            siftup_(i);
        }
    }

    //删除队尾元素
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

//向上调整堆
void HeapTimer::siftup_(int i){
    assert(i >= 0 && i < heap_.size());
    int j = (i-1)/2;
    while(j >= 0){
        // cout << j << endl;
        if(heap_[j] < heap_[i]) break;
        SwapNode_(i,j);
        i = j;
        j = (i-1) / 2;
    }
}

//向下调整堆
bool HeapTimer::siftdown_(int index,int n){
    assert(index >= 0 && index < heap_.size());
    int i = index;
    int j = i*2+1;
    while(j < n){
        if(j + 1 < n && heap_[j+1] < heap_[j]) j++;
        if(heap_[i] < heap_[j]) break;
        SwapNode_(i,j);
        i = j;
        j = i*2 + 1;
    }
    return i > index;
}

//交换两个节点
void HeapTimer::SwapNode_(size_t i,size_t j){
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i],heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
}