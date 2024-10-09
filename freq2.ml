module CharMap = Map.Make(Char)

type trie = T of trie CharMap.t * int

let rec add_word (default_val : int) (str: string) (i: int) (T (map, n): trie) = 
    if i >= String.length str then 
        (if n < 0 then T (map, -1)
        else T (map, n+1))
    else 
        T (CharMap.update str.[i] 
            (function |None -> Some (add_word default_val str (i+1) (T ( CharMap.empty, default_val)))
                      |Some t -> Some (add_word default_val str (i+1) t)) 
        map, n)
        
let add_stopword t s = add_word (-1) s 0 t
let add_bookword t s = add_word 0 s 0 t

let is_letter (c: Char.t) = (Char.code c) >= (Char.code 'a') && (Char.code c) <= (Char.code 'z')

let make_stopwords ic = 
	let str = In_channel.input_all ic in
    let str = String.lowercase_ascii str in
	String.split_on_char ',' str

let make_boowords ic = 
    let str = In_channel.input_all ic in
    let str = String.lowercase_ascii str in
    let remove_seps = function 
		|c when is_letter c -> c
      	|_ -> ' ' in
    let str = String.map remove_seps str in
    String.split_on_char ' ' str

let rec extract_topn n (T (cmap, p): trie) (cword: char list) = 
    let rec fold_func l1 l2 k = 
        if k == 0 then [] else
        match (l1, l2) with
            |((x, wx)::qx, (y, wy)::qy) -> 
                if x > y then (x, wx)::(fold_func qx ((y, wy)::qy) (k-1))
                else (y, wy)::(fold_func ((x, wx)::qx) qy (k-1))
            |([], x::q) | (x::q, []) -> x::(fold_func q [] (k-1))
            |([], []) -> []
    in
    CharMap.fold (fun c t l -> fold_func (extract_topn n t (c::cword)) l n)  cmap [(p, cword)] 
  
let () = if Array.length Sys.argv != 4 then failwith "Wrong number of arguments. Syntax is freq [book] [stopwords] [n]"
    else let n = int_of_string_opt Sys.argv.(3) in 
	match n with 
		|None -> failwith "n must be an integer"
		|Some n ->
			let stopwords = In_channel.with_open_text Sys.argv.(2) make_stopwords in
			let bookwords = In_channel.with_open_text Sys.argv.(1) make_boowords in 
            let pref_tree = (T (CharMap.empty, -1)) in
            let pref_tree = List.fold_left add_stopword pref_tree stopwords in
            let pref_tree = List.fold_left add_bookword pref_tree bookwords in
            let topn_list = extract_topn n pref_tree [] in
			List.iteri (fun i (b, a) -> Printf.printf "%d: \"%s\", with %d occurences\n" i (String.of_seq (List.to_seq (List.rev a))) b) topn_list
