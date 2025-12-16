# main.py

def add(a: int, b: int) -> int:
    """Простая функция сложения."""
    return a + b


def test_add():
    """Простой тест без использования unittest (для pytest или ручного запуска)."""
    assert add(2, 3) == 5
    assert add(-1, 1) == 0
    print("✅ Тесты для функции add прошли успешно!")


if __name__ == "__main__":
    test_add()
