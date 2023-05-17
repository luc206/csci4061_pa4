<!--test machine: csel-kh4250-01.cselabs.umn.edu (ssh)
*   group number: G19
*   name: Timothy Anderson, Luc Rubin, Hank Hanson
*   x500: and09985, rubin206, hans7186 -->

# Project #4 - Network Sockets
## Group Number 19

## Test Machine: 
`csel-kh4250-01.cselabs.umn.edu (ssh)`
## Names and x500 addresses
```
Timothy Anderson - and09985
Luc Rubin - rubin206
Hank Hanson - hans7186 
```
## Whether to complete the extra task
Yes both the history and worker thread pool was implemented

## Individual contributions
Hank worked on client.c, Luc worked on server.c, and Tim worked on utils.c. After our individual work was done, we collaborated to find bugs, cleanup the code, and made sure everything runs as expected.

## Assumptions
- `BALANCE`, `ACCOUNT_INFO`, `CASH`, and `HISTORY` are not to be expected from the input file, these messages are designed to be a response that the client recieves from the server.  If these are found in the input file they will be sent to the server as an `ERROR`
- Sometimes `Server socket bind failed...` may be printed out, this usually happens if you are running test too close together, or if the port is being used already.
- t4's expected output seems to have underscores in the name, however input4.csv does not have the underscores, this caused the diff test to fail, but the underscores are the only difference. We assummed this was a mistaken in the expected output and/or the input4.csv.

## Compile
	> make clean
	> make

## Execution
    > make run1
    > make run2 
    ...
    > make run4

### For testing the results to the expected output (requires the expected output files)
```
> make t1
> make t2
...
> make t4
```
