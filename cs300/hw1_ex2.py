import time
import numpy as np

start = time.time()

def is_sorted(l):
  if len(l)<2: return True
  for i in range(len(l)-1):
    if l[i]>l[i+1]:
      return False
  return True

def ordered_permutation(l):
  if(len(l)==0): return []
  elif (len(l)==1): return [l]
  else:
    newlist = []
    for i in range(len(l)):
      excluded = l[:i]+l[i+1:]
      for p in ordered_permutation(excluded):
        if is_sorted([l[i]]+p):
          return [[l[i]]+p]
        else:
          newlist.append([l[i]]+p)
          continue

def random_shuffle(l):
  if(len(l)==0):
    return []
  elif (len(l)==1):
    return [l]
  else:
    newlist = []
    leading_index= np.random.random_integers(0,len(l)-1)
    leading = l[leading_index]
    rest = l[:leading_index]+l[leading_index+1:]
    for p in random_shuffle(rest):
      newlist.append([leading]+p)
    return newlist

def random_bogosort(l):
  temp = l
  while not is_sorted(temp):
    temp = random_shuffle(temp)[0]
  return temp


mylist = [7,2,4,1,10,30,20,3,35]

print(random_bogosort(mylist))
print(time.time()-start)






