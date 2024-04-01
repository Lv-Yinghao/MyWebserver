class Solution {
public:
    ListNode* mergeList(vector<ListNode*>& lists,int l,int r){
        if(l==r) return lists[l];

        int mid = (l+r)/2;
        ListNode* h1 = mergeList(lists,l,mid);
        ListNode* h2 = mergeList(lists,mid+1,r);

        auto _h = new ListNode(0,nullptr);
        auto cur1 = h1,cur2 = h2,cur = _h;
        while(cur1 && cur2){
            if(cur1->val <= cur2->val){
                cur1->next = cur2;
                cur->next = cur1;
                cur1 = cur1->next;
            }
            else{
                cur2->next = cur1;
                cur->next = cur2;
                cur2 = cur2->next;
            }
            
            cur = cur->next;
        }
        
        if(cur2) cur->next = cur2;
        else cur->next = cur1;

        return _h->next;
    }
    ListNode* mergeKLists(vector<ListNode*>& lists) {
        mergeList(lists,0,lists.size()-1);
    }
};