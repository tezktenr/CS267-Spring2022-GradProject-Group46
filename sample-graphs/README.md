# Storing Graphs in TXT File
This page explains the format of specifying a graph in a text file.

## General Format
Note:
 - **V** represents the number of nodes/vertexes
    - So, **node_id** ranges from 0 to V-1
 - **E** represents the number of edges
 - Edges are specified with 3 parameters
    - 2 node ids, representing the two endpoints of the edges
    - Followed by a value of either 0 or 1 specifying the weight of the edges.
        - 1 = agreeing
        - 0 = opposing
```
V E
V1 V2 1
V2 V3 0
```

## Example

The graph above could be represented in the text file as:
```
```
