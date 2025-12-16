print("------------------------------")
def add(a: int, b: int) -> int:
    """Простая функция сложения."""
    print('start add()')
    return a + b


def test_add():
    """Простой тест без использования unittest (для pytest или ручного запуска)."""
    assert add(2, 3) == 5
    assert add(-1, 1) == 0
    print("✅ Тесты для функции add прошли успешно!")

print("start...")
# test_add()

