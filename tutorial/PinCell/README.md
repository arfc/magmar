## Pin Cell

Here, group constants are taken from an example SCALE output using MoltresXS. 
These will be used to generate an HDF5 file, which can be used by MOLTRES. 


### SCALE Input file

Here, the scale input file for a pin cell based on the (BEAVRS benchmark) 
[https://crpg.mit.edu/research/beavrs] can be found.

For group constant generation, the standard 2 group LWR structure is used.
The 'pin\_cell.scale' file contains branches and burnup that will generate group constants at different temperatures and burnup levels.
Like a cooking show, the important results have already been prepared for you in the "pin.t16" file, which
gets parsed by ``` MoltresXS.py``` using the 'pin_cell_XS.inp' file.

### MoltresXS Input file

The 'pin_cell_XS.inp' file provides ``` MoltresXS.py``` with the indices of the cross sections that should be converted to a moltres format.
The branch section is the least intuitive and requires improvement. The inputs to this section are

```[BRANCH]```
```number_of_branches```
```Material_key temperature_key file_index burnup_index universe_index branch_index```

### Command to run:

The command to run in order to generate the moltres-compatible group constants is:

```$MOLTRES/python/MoltresXS.py pin_cell_XS.inp ```

The pin cell MOLTRES model can then be run using:

```$MOLTRES/moltres-opt -i pin_cell.i ```

Where $MOLTRES is an environment variable leading to the install location of Moltres. 

The syntax requires the arbitrary directory name you'd like to create, then the arbitrary file base name that moltres will
look at, then a file that maps primary branch names to temperatures, then a file that lists the secondary branch names, and lastly a file that maps universe numbers from serpent to material names. 
Group constants will be extracted for all materials listed in the last file.

