# rrapply 1.2.2

* Added new option 'how = "bind"' to unnest a nested list to wide data.frame
* Options 'how = "flaten"', 'how = "melt"' and 'how = "bind"' coerce flat lists to common types 
* Unnamed list elements receive names "1", "2", ... in '.xname', '.xparents', 'how = "melt"' and 'how = "bind"'
* Added 'pokedex' demo dataset 
* Reorganized source code

# rrapply 1.2.1

* Arguments 'feverywhere' and 'dfaslist' are deprecated in favor of 'classes' (instead use 'classes = "list" or 'classes = "data.frame"')
* Added the option 'how = "recurse"' to replace the deprecated 'feverywhere = "recurse"'
* Cleaned up source code and fixed several minor issues

# rrapply 1.2.0

* Added support for call objects and expression vectors
* Added special argument .xsiblings evaluating to sibling list in 'f' and 'condition'

# rrapply 1.1.1

* Added new option 'how = "unmelt"' to restore nested list from melted data.frame
* Added special argument .xparents evaluating to parent node vector in 'f' and 'condition' 

# rrapply 1.1.0

* Added new option 'how = "melt"' to return melted data.frame from pruned nested list
* Added options 'feverywhere = "break"' for list node aggregation and 'feverywhere = "recurse"' for list node updating
* Cleaned up source code and fixed several minor issues

