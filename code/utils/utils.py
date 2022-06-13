from time import time
class TimeSection:
    top_level_active = False
    def __init__(self, section_name=None):
        self.section_name = section_name
        self.start_time = None
        self.end_time = None
        # If "responsible", instance can print
        self.responsible = False
    def __enter__(self):
        if not TimeSection.top_level_active:
            TimeSection.top_level_active = True
            self.responsible = True
        self.start_time = time()
        if self.responsible:
            print(f"{self.section_name}: ", end='', flush=True)
        return self
    def __exit__(self, type, value, traceback):
        self.end_time = time()
        if self.responsible:
            print(f"{self.elapsed():0.4f}sec")
            self.responsible = False
            TimeSection.top_level_active = False
    def elapsed(self):
        return self.end_time - self.start_time
