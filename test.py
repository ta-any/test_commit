# test_example_unittest.py

import unittest
print("test_example_unittest.py")
def add(a: int, b: int) -> int:
    return a + b

class TestMathOperations(unittest.TestCase):
    def test_add_positive(self):
        self.assertEqual(add(2, 3), 5)

    def test_add_with_negative(self):
        self.assertEqual(add(-1, 1), 0)

if __name__ == "__main__":
    unittest.main()
