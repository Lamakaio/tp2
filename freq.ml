module StringMap = Map.Make(String)
module StringSet = Set.Make(String)

let is_letter (c: Char.t) = (Char.code c) >= (Char.code 'a') && (Char.code c) <= (Char.code 'z')

let make_stopwords ic = 
	let str = In_channel.input_all ic in
    let str = String.lowercase_ascii str in
	let wordslist = String.split_on_char ',' str in
	let occurences : StringSet.t = StringSet.empty in
    List.fold_right StringSet.add wordslist occurences

let make_occurences (stopwords: StringSet.t) ic = 
    let str = In_channel.input_all ic in
    let str = String.lowercase_ascii str in
    let remove_seps = function 
		|c when is_letter c -> c
      	|_ -> ' ' in
    let str = String.map remove_seps str in
    let wordslist = String.split_on_char ' ' str in
	let wordslist = List.filter (fun v -> String.length v >= 2 && not (StringSet.mem v stopwords)) wordslist in
    let occurences : int StringMap.t = StringMap.empty in
    let add_to_map map v = StringMap.update v (function None -> Some 1 | Some x -> Some (x+1)) map in
    let occurences = List.fold_left add_to_map occurences wordslist in
    List.fast_sort (fun (_, i1) (_, i2) -> Int.compare i2 i1) (StringMap.to_list occurences)
  
let () = if Array.length Sys.argv != 4 then failwith "Wrong number of arguments. Syntax is freq [book] [stopwords] [n]"
    else let n = int_of_string_opt Sys.argv.(3) in 
	match n with 
		|None -> failwith "n must be an integer"
		|Some n ->
			let stopwords = In_channel.with_open_text Sys.argv.(2) make_stopwords in
			let occurences = In_channel.with_open_text Sys.argv.(1) (make_occurences stopwords) in 
			Seq.iteri (fun i (a, b) -> Printf.printf "%d: \"%s\", with %d occurences\n" i a b) (Seq.take n (List.to_seq occurences))
