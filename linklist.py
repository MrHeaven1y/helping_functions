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
    def get(self,index):
        temp_node=self.head
        for _ in range(index):
            temp_node=temp_node.next
        return temp_node.value
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
        self.length-=1
    def pop_first(self):
        pass
    def remove(self):
        pass
    def reverse(self):
        pass
    def sort(self):
        pass
    def delete_all(self):
        pass
    def set(self):
        pass
    def remove_duplicates(self):
        pass
l=LinkedList()
l.append(1)
l.append(2)
l.append(4)
l.append(5)
l.append(6)
l.append(7)
l.append(8)
l.append(9)
# l.prepend(90)
# l.insert(4,109)
# print(l.length)
# print(l)
# print(l.get(3))
# l.search(4)
# l.traverse()
l.pop()
print(l)