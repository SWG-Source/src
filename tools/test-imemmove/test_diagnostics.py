"""Safe text-only classifier tests; no C++ compilation or vendor calls."""
import unittest
from run import expected_ambiguity

class DiagnosticTests(unittest.TestCase):
    locations = [('C:/input/Misc.h', 233), ('C:/probe/probe.cpp', 29)]
    first = "C:/input/Misc.h(233) : error C2668: 'memmove' : ambiguous call to overloaded function"
    second = "C:/probe/probe.cpp(29) : error C2668: 'memmove' : ambiguous call to overloaded function"

    def accepted(self, text, exit_code=2):
        return expected_ambiguity(text, exit_code, self.locations)

    def test_complete_expected(self):
        self.assertTrue(self.accepted(self.first + '\n' + self.second))
    def test_normalized_windows_paths(self):
        self.assertTrue(self.accepted(self.first.replace('C:/input/', 'c:\\INPUT\\sub\\..\\') + '\n' + self.second))
    def test_reordered(self):
        self.assertTrue(self.accepted(self.second + '\n' + self.first))
    def test_expected_plus_unrelated(self):
        self.assertFalse(self.accepted(self.first + '\n' + self.second + '\nx.cpp(1) : error C2065: missing symbol'))
    def test_unrelated_ambiguity_incidental_memmove(self):
        self.assertFalse(self.accepted("note: memmove\nx.cpp(1) : error C2668: 'other' : ambiguous call to overloaded function"))
    def test_wrong_path_same_basename(self):
        self.assertFalse(self.accepted(self.first.replace('input', 'other') + '\n' + self.second))
    def test_wrong_line(self):
        self.assertFalse(self.accepted(self.first.replace('(233)', '(234)') + '\n' + self.second))
    def test_wrong_function(self):
        self.assertFalse(self.accepted(self.first.replace("'memmove'", "'other'") + '\n' + self.second))
    def test_wrong_code(self):
        self.assertFalse(self.accepted(self.first.replace('C2668', 'C2065') + '\n' + self.second))
    def test_missing_expected_record(self):
        self.assertFalse(self.accepted(self.first))
    def test_duplicate_record(self):
        self.assertFalse(self.accepted(self.first + '\n' + self.second + '\n' + self.first))
    def test_no_errors(self):
        self.assertFalse(self.accepted('compiler banner\nmemmove'))
    def test_success_exit(self):
        self.assertFalse(self.accepted(self.first + '\n' + self.second, 0))
    def test_fatal_and_link_errors(self):
        for error in ['fatal error C1083: missing header', 'LINK : fatal error LNK1120: unresolved', 'error: unrecognized failure']:
            self.assertFalse(self.accepted(self.first + '\n' + self.second + '\n' + error))

if __name__ == '__main__':
    unittest.main()
