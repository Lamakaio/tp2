import sys
import re

assert(len(sys.argv) == 4, "Wrong number of arguments. Syntax is freq [book] [stopwords] [n]")
try: 
    n = int(sys.argv[3])
except:
    raise TypeError("n must be an integer")

stopwords_path = sys.argv[2]
book_path = sys.argv[1]

def make_stopword_dict(filename):
    stopwords = set()
    with open(filename, "r") as f:
        s = f.read()
        stopwords.update(s.split(","))
    
    return stopwords

def calc_occurences(filename, stopwords):
    with open(filename, "r") as f:
        s = f.read()
        wordslist = re.split("[^\w]", s.lower())
        finaldict = {}
        for w in wordslist: 
            if len(w) > 1 and w not in stopwords:
                finaldict.setdefault(w, 0)
                finaldict[w] += 1
        return finaldict

stopwords = make_stopword_dict(stopwords_path)

occurences = calc_occurences(book_path, stopwords)

for (k, (i, j)) in enumerate(sorted(occurences.items(), key = lambda x: x[1], reverse=True)[:n]):
    print(f"""{k+1}: "{i}", with {j} occurences""")

