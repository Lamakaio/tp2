stopwords_path = "TPs/tp2/stop_words.txt"
book_path = "TPs/tp2/pride_prejudice.txt"
separators = [",", "(", ")", ".", ";", "\n", "[", "]", "-", "\"", '”', '“']

def make_stopword_dict(filename):
    stopwords = set()
    with open(filename, "r") as f:
        s = f.read()
        stopwords.update(s.split(","))
    return stopwords

def calc_occurences(filename, stopwords):
    with open(filename, "r") as f:
        translate_dict = {s: " " for s in separators}
        s = f.read()
        wordslist = s.lower().translate(str.maketrans(translate_dict)).split(" ")
        finaldict = {}
        for w in wordslist: 
            if w != '' and w not in stopwords:
                finaldict.setdefault(w, 0)
                finaldict[w] += 1
        return finaldict

stopwords = make_stopword_dict(stopwords_path)

occurences = calc_occurences(book_path, stopwords)

print(sorted(occurences.items(), key = lambda x: x[1], reverse=True)[:20])

