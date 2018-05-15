#! /bin/python2

from pdb import set_trace
from errno import * 

dir_list = {
    '/usr/share/doc/fuse-python/example/': True, 
    '/usr/share/doc/fuse-python/': True, 
    '/usr/share/doc/': True,
    '/usr/share/': True,
    '/usr/': True,
    '/': True
}

test1 = '/usr/share/doc/fuse-python/example/hello.py'
test2 = '/usr/share/doc/'
test3 = '/' 
test4 = '/test/etc/'
test5 = '/usr/share/test/example/'

set_trace()

def test_dir_search(path):
    # generate the parent
    if path.endswith('/'):
        parent = path[:path.rfind('/', 0, len(path) - 1) + 1]
    else:
        parent = path[:path.rfind('/') + 1]

    # Check if the parent exists
    if not dir_list.has_key(parent):
        print 'ERROR: path does not exist'
        return ENOENT

        
    print 'found path: %s' % path
    
    
if __name__ == "__main__":
    test_dir_search(test1)
    test_dir_search(test2)
    test_dir_search(test3)
    test_dir_search(test4)
    test_dir_search(test5)