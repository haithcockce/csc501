def try_to_import():
    import pdb
    global pdb

def debug_stuff():
    try_to_import()
    try:
        pdb.set_trace()
    except NameError:
        print "Couldn't find it"

if __name__ == "__main__":
    debug_stuff()
