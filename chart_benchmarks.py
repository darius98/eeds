#!/usr/bin/env python3

from argparse import ArgumentParser
import json
from math import sqrt
from pathlib import Path
from typing import Dict, Any, Tuple, List
import matplotlib.pyplot as plt

opts: Any = None
g_context = None


def check_context(context: Dict[str, Any]):
    global g_context
    if g_context is None:
        g_context = context
        return
    checked_keys = [
        "host_name",
        "num_cpus",
        "mhz_per_cpu",
        "cpu_scaling_enabled",
        "library_build_type",
    ]
    for key in checked_keys:
        if g_context[key] != context[key]:
            print("WARNING: mismatched Google Benchmark context: ",
                  key,
                  ": ",
                  g_context[key],
                  " != ",
                  context[key],
                  sep="")


class ChartData:
    COLORS = ["blue", "gray", "darkgray", "lightgray"]

    def __init__(self, category: str, sub_category: str):
        self.category = category
        self.subcategory = sub_category
        self.chart_lines: Dict[str, List[Tuple[str, float]]] = {}

    def add_entry(self, benchmark_name: str, typename: str, size: str, real_time: float):
        if opts.max < 0 or int(size) <= opts.max:
            chart_line_key = typename + " from " + benchmark_name
            self.chart_lines.setdefault(chart_line_key, [])
            self.chart_lines.get(chart_line_key).append((size, real_time))

    def render(self, plot):
        for chart_line in self.chart_lines.values():
            chart_line.sort(key=lambda p: int(p[0]))

        diff = 0
        if len(self.chart_lines) >= 2:
            first_line = sum(map(lambda p: p[1], list(self.chart_lines.values())[0]))
            second_line = sum(map(lambda p: p[1], list(self.chart_lines.values())[1]))
            diff = first_line / second_line * 100 - 100

        d = ""
        if diff:
            d = " " + ("+" if diff > 0 else "") + str(round(diff)) + "%"
        plot.set_title(self.category + "_" + self.subcategory + d)
        plot.title.set_color("green" if diff < -3 else "red" if diff > 3 else "black")

        i = 0
        for chart_line in self.chart_lines.values():
            chart_line.sort(key=lambda p: int(p[0]))
            xs, ys = [], []
            for entry in chart_line:
                xs.append(entry[0])
                ys.append(entry[1])
            plot.plot(xs, ys, color=self.COLORS[i])
            i = (i + 1) % len(self.COLORS)


def add_data(data: Dict[str, ChartData], benchmark_name: str, benchmark_entry: Dict[str, Any], add: bool):
    category, sub_category, typename, size = benchmark_entry["name"].split("/")
    real_time = benchmark_entry["real_time"]
    key = category + "/" + sub_category
    if add:
        data.setdefault(key, ChartData(category, sub_category))
    data.get(key, ChartData("", "")).add_entry(benchmark_name, typename, size, real_time)


def parse_data(data: Dict[str, ChartData], filepath: str, add: bool):
    filename = Path(filepath).stem
    with open(filepath, "r") as f:
        file_data = json.load(f)
        check_context(file_data["context"])
        for entry in file_data["benchmarks"]:
            add_data(data, filename, entry, add)


def main():
    parser = ArgumentParser()
    parser.add_argument("--ref", action="append", dest="ref", type=str, default=[],
                        help="Reference files")
    parser.add_argument("--max", type=int, default=-1,
                        help="Display only with sizes up to this value. If <0, display all available data")
    parser.add_argument("file", help="Target file")

    global opts
    opts = parser.parse_args()

    data: Dict[str, ChartData] = {}

    parse_data(data, opts.file, True)
    for ref_file in opts.ref:
        parse_data(data, ref_file, False)

    # Find the best num lines
    num_lines = 1
    num_columns = len(data)
    for i in range(int(sqrt(len(data))), len(data)):
        if len(data) % i == 0:
            num_lines = i
            num_columns = len(data) // i
            break

    fig, plots = plt.subplots(num_lines, num_columns, figsize=(36, 18))
    i = 0
    j = 0
    for chart_data in data.values():
        chart_data.render(plots[i, j])

        j += 1
        if j == num_columns:
            i += 1
            j = 0

    plt.show()


if __name__ == "__main__":
    main()
