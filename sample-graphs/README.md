# Storing Graphs in TXT File
This page explains the format of specifying a graph in a text file.

## General Format
```
V E
V1 V2 1
V2 V3 0
...
```
The first line specifies the number of vertexes and the number of edges. Each of the line after the first line specify an edge. The edge should have no direction and should be unique.

Note:
 - **V** represents the number of nodes/vertexes
    - So, **node_id** ranges from 0 to V-1
 - **E** represents the number of edges
 - Edges are specified with 3 parameters
    - 2 node ids, representing the two endpoints of the edges
    - Followed by a value of either 0 or 1 specifying the weight of the edges.
        - 1 = agreeing
        - 0 = opposing

## Example
![alt text](https://github.com/tezktenr/CS267-Spring2022-GradProject-Group46/blob/main/sample-graphs/pic_sample_graph.png "Sample Graph Picture")

The graph above could be represented in the text file as:

```
4 5
0 1 1
0 2 1
2 3 1
0 3 0
1 3 1
```
