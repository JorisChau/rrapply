# rrapply 1.2.5

* **Breaking change**: data.frame columns in `how = "bind"` only include child list names 
  instead of full path names
* Added new option `how = "names"` to recursively update list names
* Added new argument `options` to control default settings for `how`. Includes control parameters:
  - `namesep`, separator to combine list names in `how = "flatten"` and `how = "bind"`;
  - `simplify`, coerce flattened list to vector in `how = "flatten"` and `how = "melt"`;
  - `namecols`, include parent names as columns `L1`, `L2`, ... in wide data.frame with `how = "bind"`;
  - `coldepth`, override depth layer mapping list elements to data.frame columns with `how = "bind"`

# rrapply 1.2.4

* Fixed minor issue with nested data.frames using `classes = "data.frame"` and `how = "recurse"`
* Removed deprecated arguments `feverywhere` and `dfaslist`

# rrapply 1.2.3

* Fixed a minor bug `classes` argument in case of missing `f` and `condition` arguments

# rrapply 1.2.2

* Added new option `how = "bind"` to unnest a nested list to wide data.frame
* Options `how = "flatten"`, `how = "melt"` and `how = "bind"` coerce flat lists to common types 
* Unnamed list elements receive names `"1"`, `"2"`, ... in `.xname`, `.xparents`, `how = "melt"` and `how = "bind"`
* Added `pokedex` demo dataset 
* Reorganized source code

# rrapply 1.2.1

* Arguments `feverywhere` and `dfaslist` are deprecated in favor of `classes` (instead use `classes = "list"` or `classes = "data.frame"`)
* Added the option `how = "recurse"` to replace the deprecated `feverywhere = "recurse"`
* Cleaned up source code and fixed several minor issues

# rrapply 1.2.0

* Added support for call objects and expression vectors
* Added special argument `.xsiblings` evaluating to sibling list in `f` and `condition`

# rrapply 1.1.1

* Added new option `how = "unmelt"` to restore nested list from melted data.frame
* Added special argument `.xparents` evaluating to parent node vector in `f` and `condition` 

# rrapply 1.1.0

* Added new option `how = "melt"` to return melted data.frame from pruned nested list
* Added options `feverywhere = "break"` for list node aggregation and `feverywhere = "recurse"` for list node updating
* Cleaned up source code and fixed several minor issues

