#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

// Function to get the maximum value in an array
int getMax(const vector<int>& arr) {
    return *max_element(arr.begin(), arr.end());
}

// Using counting sort to sort the elements based on significant places
void countingSort(vector<int>& arr, int exp) {
    const int n = arr.size();
    vector<int> output(n, 0);
    vector<int> count(10, 0);

    // Count occurrences of elements
    for (int i = 0; i < n; i++)
        count[(arr[i] / exp) % 10]++;

    // Modify count to store actual position of this digit in output
    for (int i = 1; i < 10; i++)
        count[i] += count[i - 1];

    // Build the output array
    for (int i = n - 1; i >= 0; i--) {
        output[count[(arr[i] / exp) % 10] - 1] = arr[i];
        count[(arr[i] / exp) % 10]--;
    }

    // Copy the output array to arr so that arr now contains sorted numbers according to current digit
    copy(output.begin(), output.end(), arr.begin());
}

// Radix Sort function
void radixSort(vector<int>& arr) {
    int maxVal = getMax(arr);

    // Iterate through all digits
    for (int exp = 1; maxVal / exp > 0; exp *= 10)
        countingSort(arr, exp);
}

// Function to print an array
void printArray(const vector<int>& arr) {
    for (int num : arr)
        cout << num << " ";
    cout << endl;
}

int main() {
    vector<int> arr = {170, 45, 75, 90, 802, 24, 2, 66};

    cout << "Original Array: ";
    printArray(arr);

    radixSort(arr);

    cout << "Sorted Array: ";
    printArray(arr);

    return 0;
}
