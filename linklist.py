class Node:
    def __init__(self,value):
        self.value=value
        self.next = None

class LinkedList:
    def __init__(self):
        self.head=None
        self.tail=None
        self.length=0
    def __str__(self):
        result=''
        temp_node=self.head
        if temp_node is None:
            return result
        else:
            while temp_node:
                result+=str(temp_node.value)
                if temp_node.next is not None:
                    result+='->'
                temp_node=temp_node.next
            return result
    def append(self,value):
        new_node=Node(value)
        if self.head is None:
            self.head=new_node
            self.tail=new_node
        else:
            self.tail.next=new_node
            self.tail=self.tail.next
        self.length+=1

    def prepend(self,value):
        new_node=Node(value)
        if self.head is None:
            self.head=new_node
            self.tail=new_node
        else:
            new_node.next=self.head
            self.head=new_node
        self.length+=1
    def insert(self,index,value):
        new_node=Node(value)
        temp_node=self.head
        if index==0:
            self.prepend(value)
        if index== -1:
            self.append(value)
        if index<-1 or index>=self.length:
            raise Exception('Index Out of list range')
        else:
            for _ in range(index-1):
                temp_node=temp_node.next
            new_node.next=temp_node.next
            temp_node.next=new_node
            self.length += 1
    def get(self,index,node=False):
        temp_node=self.head
        for _ in range(index):
            temp_node=temp_node.next
        if node:
            return temp_node
        return temp_node.value
    def set(self,index,value):
        new_node=Node(value)
        if index==0:
            new_node.next=self.head.next
            self.head=new_node
        if index==-1:
            # prev_node = self.get(self.length - 2, node=True)
            # prev_node.next = new_node
            # self.tail=new_node
            pass
            # current_node = self.tail
            # current_node.next=None
        if index<-1 or index>=self.length:
            raise Exception("Index out of range of list bound")
        else:
            prev_node=self.get(index-1,node=True)
            current_node=self.get(index,node=True)
            prev_node.next=new_node
            new_node.next=current_node.next
            current_node.next=None
    def search(self,target):
        temp_node=self.head
        count=0
        while temp_node:
            if temp_node.value==target:
                print(f"found at a index of {count}")
                return None
            count+=1
            temp_node=temp_node.next
        return None
    def traverse(self):
        current=self.head
        print(current.value)
        while (current :=current.next) !=None:
            print(current.value)
    def pop(self):
        poped_node=self.tail
        if self.length==0:
            return None
        if self.length==1:
            self.head=self.tail=None
        else:
            temp_node=self.head
            while temp_node.next is not self.tail:
                temp_node=temp_node.next
            self.tail=temp_node
            temp_node.next=None
            return poped_node.value
        self.length-=1
    def pop_first(self):
        if self.length==0:
            return None
        pop_node=self.head
        temp=pop_node.next
        self.head=temp
        self.length-=1
        return pop_node.value
    def remove(self,index):
        if index==0:
            self.pop_first()
        if index==-1:
            self.pop()
        if index<-1 or index>=self.length:
            raise Exception("index out of range")
        else:
            temp_node=self.head
            for _ in range(index-1):
                temp_node=temp_node.next
            pop_node=temp_node.next
            temp_node.next=pop_node.next
            pop_node.next=None
            self.length-=1
            return pop_node.value

    def reverse(self):
        current = self.head
        prev_node = None
        while current:
            next_node = current.next
            current.next = prev_node
            prev_node = current
            current = next_node
        self.head,self.tail=self.tail,self.head
    def delete_all(self):
        self.head=self.tail=None
        self.length=0
    def sort(self):
       pass
    def remove_duplicates(self):
        current=self.head
        if current is None:
            return None
        node_values=set()
        node_values.add(current.value)
        while current.next:
            if current.next.value in node_values:
                current.next=current.next.next
                self.length-=1
            else:
                node_values.add(current.next.value)
                current=current.next
        self.tail=current
