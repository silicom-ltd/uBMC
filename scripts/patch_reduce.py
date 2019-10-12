#!/usr/bin/python
import sys

pstate_hdr_wait = 0
pstate_patch_wait = 1

class fmtError(Exception):
    def __init__(self, str, l):
        sys.stderr.write("%s\nLine:\n%s" %(str, l))
        
class patch_file():
    def __init__(self, hdr_lines):
        self.hdr = hdr_lines
        self.patches = []
        self.src_file = hdr_lines[2].split(' ')[1].split('\t', 1)[0]
        self.dst_file = hdr_lines[3].split(' ')[1].split('\t', 1)[0]
        self.file = self.src_file.split('/', 1)[1]
        tmp = self.dst_file.split('/', 1)[1]
        if self.file != tmp:
            raise fmtError("src and dst file not the same? %s %s\n" %(self.file, tmp), hdr_lines[3])
        #sys.stderr.write("Patches for file %s\n"%self.file)
    def add(self, patch):
        self.patches.append(patch)
    def __str__(self):
        return "Patch of %s"%self.file

class patch():
    def __init__(self, line_info):
        try:
            self.line_info = line_info
            parts = line_info.split(' ')
            src=parts[1].split('-')[1].split(',')
            dst=parts[2].split('+')[1].split(',')
            self.src_start = int(src[0])
            self.src_lines = int(src[1])
            self.dst_start = int(dst[0])
            self.dst_lines = int(dst[1])
            self.content = []
            #sys.stderr.write("Patch Section, src %u:%u dst %u:%u\n" %(self.src_start, self.src_lines, self.dst_start, self.dst_lines))
        except:
            raise fmtError("Patch section header error", line_info)
    def diff_hdr_change_only(self):
        for l in self.content:
            if l[0] == ' ':
                continue;
            if l[0] == '-' or l[1] == '+' :
                if l[1:4] != '+++' and l[1:4] != '---':
                    if l[1:10] != 'diff -uNr':
                        #sys.stderr.write("line is not differ header change, %s\n\t%s"%(l[1:3],l))
                        return False
                
        return True
    def __str__(self):
        return self.line_info
def print_help():
    sys.stderr.write("Usage %s <patch_file> <reverse_ouput_patch>\n" % sys.argv[0])

def line_check(l, match_str):
    if l.find(match_str) != 0:
        raise fmtError("Expecting '%s' at beginning of file" % match_str, l);
    "Expecting  at beginning of file"
    
def check_patch(lines):
    state =  pstate_hdr_wait;
    ret = []
    pf = None
    new_patch = None
    for l in lines:
        if l[0:7] == "Index: ":
            if(pf!=None):
                if new_patch != None:
                    pf.add(new_patch)
                    new_patch = None
                ret.append(pf)
                pf = None
            if new_patch != None:
                raise fmtError("No patch file 'Index:' found", l);
            hdr_lines = []
            hdr_lines.append(l)
            l = lines.next()
            line_check(l, "=======================")
            hdr_lines.append(l)
            l = lines.next()
            line_check(l, "--- ")
            hdr_lines.append(l)
            l = lines.next()
            line_check(l, "+++ ")
            hdr_lines.append(l)
            pf = patch_file(hdr_lines)
        elif l[0:3] == "@@ ":
            if pf == None:
                raise fmtError("No patch file 'Index:' found", l);
            if new_patch != None:
                pf.add(new_patch)
            new_patch = patch(l)
        else:
            if new_patch == None:
                raise fmtError("No patch secstion start @@ found", l);
            if pf == None:
                raise fmtError("No patch file 'Index:' found", l);
            if l[0] == '-' or l[0] == ' ' or l[0] == '+':
                new_patch.content.append(l)
            else:
                raise fmtError("Invalid patch line", l)
    if pf != None:
        if new_patch != None:
            pf.add(new_patch)
        ret.append(pf)
    return ret


def main():
    try:
        patch_file = file(sys.argv[1], 'r')
        out_file = file(sys.argv[2], 'w')
    except IndexError:
        print_help()
        exit(1)
    patch_file_list = check_patch(iter(patch_file))
    
    for patch_file in patch_file_list:
        diff_hdr_change = []
        #print "Checking file %s"% patch_file
        for patch in patch_file.patches:
            if patch.diff_hdr_change_only():
                diff_hdr_change.append(patch)
                print("\t\t%s[%s] will be removed"%(patch_file.file, str(patch)[0:-2]))
        if len(diff_hdr_change) != 0:
            for l in patch_file.hdr:
                out_file.write(l)
            for patch in diff_hdr_change:
                out_file.write(patch.line_info)
                for l in patch.content:
                    out_file.write(l)
    out_file.close()
    

if __name__ == "__main__":
    main()