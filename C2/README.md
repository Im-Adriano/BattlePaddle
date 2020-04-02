
# C2

## What is it?

A C2 for BP...

## Running/Installation?

First install the requirements with 

```bash
python -m pip install -r requirements.txt
```

Then run the C2 with:
```bash
python C2.py
```

If the chosen port BP is communicating on is in the reserved range you will need to run the C2 as root or administrator.

## How to use?

Example workflow:

```python
set name <Enter Name of this command wave>
set target list <list of target IPs separated by spaces>
set command <Enter command to run>  # This can be ran as many times as you want to stage many command
stage
execute <Name you entered above>
```

### Creating and targeting groups

```python
# Creating a group of target IPs
create group <group name here>
edit group <same group name here>
add <list of target IPs separated by spaces>

# Creating command wave to target new group
set name <Enter Name of this command wave>
set target group <list of groups separated by spaces>
set command <Enter command to run> # This can be ran as many times as you want to stage many command
stage
execute <Name you entered above>
```

### Viewing target responses

```
show responses target <IP to view>
```

