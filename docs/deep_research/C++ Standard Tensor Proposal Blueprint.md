# **P3500R0: Standardizing std::tensor for Deep Learning and High-Performance Data-Intensive Computing in ISO C++**

## **Executive Summary and Problem Statement**

In the Python data science ecosystem, the numpy.ndarray object serves as the fundamental interchange format for numerical computing. Higher-level frameworks—including PyTorch, TensorFlow, JAX, SciPy, and OpenCV—build directly upon NumPy's unified concept of contiguous and strided multidimensional array buffers. This shared foundation enables seamless, zero-copy data exchange across heterogeneous software boundaries.  
In contrast, the C++ software landscape features a fragmented ecosystem of incompatible multidimensional array and tensor implementations. High-performance software engineering in C++ relies on disconnected third-party abstractions, such as Eigen Tensor, LibTorch at::Tensor, TensorFlow Core Tensor, OpenCV cv::Mat, Armadillo, Blaze, and xtensor. Because these libraries do not share a common standard owning container type, integrating components across C++ library boundaries introduces significant software overhead1.  
Passing multidimensional array data between disparate C++ third-party libraries currently forces developers to choose between two sub-optimal architectures:

1. **Deep Memory Copies**: Allocating new memory buffers and copying element data across library boundaries introduces significant memory bandwidth penalties and execution latency1. In deep learning pipelines where data-intensive tensor transformations execute continuously across host and accelerator devices, deep copying degrades operational throughput1.  
2. **Opaque Raw Pointer Casting**: Bypassing type safety by extracting raw data pointers (such as void\* or float\*) strips away crucial metadata, including dynamic shape extents, stride configurations, element data types, layout policies, and ownership semantics2. This approach compromises compile-time type safety, increases the risk of memory leaks and lifetime errors, and breaks thread safety invariants2.

While recent C++ standards have introduced essential non-owning multidimensional abstractions—such as std::mdspan in C++235 and dense linear algebra algorithms in std::linalg for C++266—the Standard Template Library (STL) still lacks a universal, owning, dynamic multidimensional container. Proposed owning adapters such as std::mdarray (P1684 / P3308) provide valuable container wrappers, but they enforce static rank constraints at compile time, lack native awareness of heterogeneous hardware memory spaces (such as CUDA, ROCm, and SYCL memory domains), and omit standard Application Binary Interface (ABI) protocols for zero-copy cross-language exchange2.  
This paper presents a formal design blueprint for std::tensor, a proposed addition to the ISO C++ Standard Template Library under header \<tensor\>. Designed as a multidimensional, heterogeneous-aware container abstraction, std::tensor bridges compile-time layout optimizations with dynamic runtime rank flexibility. It integrates directly with existing C++ facilities while establishing a standard C ABI bridge based on the established DLPack protocol (DLManagedTensorVersioned) for zero-copy data exchange across C++ libraries, Python runtimes, and hardware accelerators2.

## **Architectural Analysis of ISO C++ Multidimensional Primitives**

Evaluating the design space for std::tensor requires examining the capabilities and structural limitations of existing and proposed ISO C++ multidimensional array facilities.

                  ┌─────────────────────────────────────────────────────────┐  
                  │                       std::tensor                       │  
                  │   \- Owning Multidimensional Container                   │  
                  │   \- Dynamic and Static Rank Support                     │  
                  │   \- Shared Storage Control Block & PMR Allocators       │  
                  │   \- Heterogeneous Memory Domain Awareness               │  
                  └────────────────────────────┬────────────────────────────┘  
                                               │  
                       ┌───────────────────────┴───────────────────────┐  
                       │                                               │  
                       ▼                                               ▼  
    ┌────────────────────────────────────┐          ┌────────────────────────────────────┐  
    │          std::mdspan View          │          │      DLManagedTensorVersioned      │  
    │  \- Non-Owning View Reference       │          │  \- Native C ABI Exchange Protocol  │  
    │  \- Direct std::linalg Integration  │          │  \- Zero-Copy Cross-Language Bridge │  
    └────────────────────────────────────┘          └────────────────────────────────────┘

### **The Non-Owning Abstraction: std::mdspan**

Standardized in C++23 via P0009, std::mdspan provides a non-owning multidimensional view over a contiguous or strided sequence of elements5. The architecture of std::mdspan decouples multidimensional indexing from element storage through four template parameters5:

1. **Element Type (ElementType)**: Defines the object type stored in the underlying sequence5.  
2. **Extents (Extents)**: Represents the multidimensional index space domain using a combination of compile-time static extents (std::static\_extent) and runtime dynamic extents (std::dynamic\_extent)5.  
3. **Layout Policy (LayoutPolicy)**: Maps a multidimensional index tuple ![][image1] to a 1D scalar memory offset5. Standard policies include layout\_left (column-major), layout\_right (row-major), layout\_stride (arbitrary striding), and padded variations (layout\_left\_padded, layout\_right\_padded via P2642)5.  
4. **Accessor Policy (AccessorPolicy)**: Governs element dereferencing semantics, enabling default pointer dereferencing, overaligned SIMD accessors (aligned\_accessor via P2897), or specialized atomic accessors5.

Although std::mdspan serves as an efficient viewing abstraction, it does not manage storage memory5. It relies on external memory allocations whose lifetimes must exceed that of the mdspan instance.

### **Slicing and Views: submdspan**

Proposal P2630 (submdspan) introduces subview extraction capabilities for std::mdspan13. By passing slice specifiers—such as full range markers (full\_extent), scalar indices, index pairs, or strided range descriptors (strided\_slice)—developers can generate sub-dimensional views without altering underlying element storage13.  
Subsequent enhancements (P3355, P3663) extend submdspan to support user-defined pair-like slice types and preserve compile-time layout properties, preventing structured layouts (e.g., layout\_left\_padded) from needlessly devolving into generic layout\_stride mappings11.

### **Algorithmic Linear Algebra: std::linalg**

Proposal P1673 introduces a set of free-function algorithms for matrix and vector arithmetic operating directly on std::mdspan views6. It standardizes classic BLAS Level 1, 2, and 3 operations—such as matrix\_vector\_product and matrix\_product—within the standard library7.  
Because std::linalg functions accept std::mdspan parameters, they are storage-agnostic5. However, this non-owning design requires callers to separately allocate and manage output memory buffers, underscoring the need for a complementary owning container abstraction5.

### **Owning Container Adapters: std::mdarray**

To provide an owning counterpart to std::mdspan, proposal P1684 (updated in P3308) introduces std::mdarray, an owning multidimensional container adapter8. std::mdarray wraps a 1D sequential container—such as std::vector\<T\> or std::array\<T, N\>—and mirrors the Extents and LayoutPolicy interfaces of mdspan8.  
Despite its utility as a general container adapter, std::mdarray exhibits key structural limitations when applied to data-intensive deep learning workloads:

* **Static Rank Enforcement**: The rank ![][image2] of std::mdarray is fixed at compile time via its Extents template argument8. Deep learning workloads frequently require dynamic runtime ranks, where tensor dimensionality varies across computational graph layers or input batches2.  
* **Container Adapter Overhead**: Delegate storage management to an underlying 1D container introduces ambiguities around moved-from object states, allocation alignment, and restrictive capacity models8.  
* **Absence of Heterogeneous Memory Awareness**: std::mdarray does not account for execution domains across CPU host memory, CUDA device memory, ROCm host-pinned memory, or SYCL unified memory spaces2.  
* **Lack of Standard C ABI Interoperability**: std::mdarray lacks built-in support for zero-copy binary data exchange with external C APIs, Python runtimes, or C-based foreign function interfaces (FFIs)2.

| Multidimensional Abstraction | Ownership Model | Rank Determination | Memory Layout Support | Heterogeneous Memory Support | Standard C ABI Exchange Protocol |
| :---- | :---- | :---- | :---- | :---- | :---- |
| std::mdspan \[cite: 5\] | Non-owning View | Static Rank (Compile-time ![][image2]) | left, right, stride, padded \[cite: 5, 11\] | Implicit via AccessorPolicy \[cite: 5, 10\] | Unstandardized |
| std::mdarray \[cite: 8\] | Owning Adapter | Static Rank (Compile-time ![][image2]) | left, right, stride \[cite: 8, 19\] | Allocator-dependent8 | Unstandardized |
| Eigen Tensor | Owning / View | Static Rank (Compile-time ![][image2]) | Row-Major / Column-Major | Host CPU & CUDA Device | Custom non-standard C++ API |
| PyTorch at::Tensor | Owning (Ref-counted) | Dynamic Rank (Runtime ![][image2]) | Strided / Non-contiguous | Host CPU, CUDA, MPS, XLA | Native DLPack Protocol2 |
| **Proposed std::tensor** | **Owning / Shared** | **Hybrid (Static ![][image2] or Dynamic)** | **left, right, stride, padded** | **Explicit device\_context** | **Native DLManagedTensorVersioned** \[cite: 2, 4\] |

## **Technical Design Blueprint for std::tensor**

The proposed std::tensor class template balances compile-time layout optimization with dynamic runtime rank capabilities. It acts as an owning multidimensional container while maintaining native interoperability with std::mdspan and std::linalg5.

### **Class Template Architecture and Signatures**

Defined within the \<tensor\> header, std::tensor is structured as a specialization of std::basic\_tensor:

C++  
namespace std {

// Special tag type designating dynamic runtime rank determination  
struct dynamic\_rank\_t { explicit dynamic\_rank\_t() \= default; };  
inline constexpr dynamic\_rank\_t dynamic\_rank{};

template \<  
    typename ElementType,  
    typename ExtentsPolicy,  
    typename LayoutPolicy \= std::layout\_right,  
    typename Allocator \= std::allocator\<ElementType\>  
\>  
class basic\_tensor;

// Type alias for statically ranked tensors (rank fixed at compile time)  
template \<  
    typename ElementType,  
    typename Extents,  
    typename LayoutPolicy \= std::layout\_right,  
    typename Allocator \= std::allocator\<ElementType\>  
\>  
using tensor \= basic\_tensor\<ElementType, Extents, LayoutPolicy, Allocator\>;

// Type alias for dynamically ranked tensors (rank determined at runtime)  
template \<  
    typename ElementType,  
    typename LayoutPolicy \= std::layout\_right,  
    typename Allocator \= std::allocator\<ElementType\>  
\>  
using dynamic\_tensor \= basic\_tensor\<ElementType, dynamic\_rank\_t, LayoutPolicy, Allocator\>;

} // namespace std

### **Static vs. Dynamic Rank Mechanics**

To support both fixed-rank mathematical operations and flexible runtime deep learning pipelines, ExtentsPolicy operates in two distinct modes:

1. **Static Rank Mode**: ExtentsPolicy is an instance of std::extents\<IndexType, Extents...\>5. The rank ![][image2] is fixed at compile time (Extents::rank()), while individual dimension extents may be static (std::static\_extent) or dynamic (std::dynamic\_extent)5.  
2. **Dynamic Rank Mode**: ExtentsPolicy is specified as std::dynamic\_rank\_t. The rank ![][image2] is determined at construction time and stored in a lightweight runtime container (such as a stack-optimized small\_vector\<size\_t, 8\>). Striding and index calculation equations dynamically adapt to rank modifications without requiring template recompilation.

### **Striding Calculations and Memory Offset Equations**

For a tensor of rank ![][image3] with shape dimensions ![][image4], the offset calculation for an index tuple ![][image5] is governed by the active LayoutPolicy5:

* **Row-Major Layout (std::layout\_right)**:  
  ![][image6]  
  ![][image7]  
* **Column-Major Layout (std::layout\_left)**:  
  ![][image8]  
  ![][image7]  
* **General Strided Layout (std::layout\_stride)**: Explicitly stores per-axis stride values ![][image9], allowing non-contiguous subviews, transposed aliases, and zero-stride broadcast projections13.

### **Storage Architecture and Reference-Counted Shared Memory**

std::tensor manages memory allocations using a reference-counted storage handle (such as a polymorphic memory resource control block). This design provides several operational capabilities:

1. **Shallow Copy Slicing (![][image10])**: Slicing, reshaping, or transposing operations via submdspan return new std::tensor instances that share the underlying data buffer while maintaining independent index mapping metadata12.  
2. **Explicit Deep Copy Cloning**: Deep memory copying is isolated to explicit tensor::clone() operations, preventing accidental heavy allocations during function parameter passing.  
3. **PMR Allocator Integration**: Support for std::pmr::memory\_resource allows integration with custom allocation strategies, including arena allocators, memory pools, and host-pinned memory resources8.

### **Heterogeneous Execution Domain Awareness**

Deep learning software executes across heterogeneous hardware domains, including CPU host memory, CUDA GPU allocations, ROCm device memory, and SYCL unified shared memory spaces2. std::tensor incorporates explicit execution domain tracking into its storage model:

C++  
namespace std {

enum class device\_type\_t : int32\_t {  
    cpu \= 1,  
    cuda \= 2,  
    cuda\_host \= 3,  
    opencl \= 4,  
    vulkan \= 7,  
    rocm \= 10,  
    rocm\_host \= 11,  
    sycl \= 12  
};

struct device\_context {  
    device\_type\_t device\_type{device\_type\_t::cpu};  
    int32\_t device\_id{0};  
};

} // namespace std

Each tensor instance stores a device\_context descriptor alongside its memory handle. Attempting to directly dereference host pointers for GPU-resident tensors raises a runtime exception or triggers precondition violations under library hardening checks2.

## **Interoperability Engine: Native C ABI Exchange via DLPack**

To eliminate C++ tensor library fragmentation, std::tensor incorporates a native C ABI export and import engine based on the standard DLPack specification (DLManagedTensorVersioned)2.

### **Standard C ABI Data Structures**

The DLPack standard specifies C ABI data structures designed for zero-copy tensor exchange across frameworks and runtime boundaries2:

C  
// Plain C Tensor Descriptor (non-owning layout ABI struct)  
typedef struct {  
    void\* data;  
    DLDevice device;  
    int32\_t ndim;  
    DLDataType dtype;  
    int64\_t\* shape;  
    int64\_t\* strides;  
    uint64\_t byte\_offset;  
} DLTensor;

// Versioned Managed Tensor C ABI Structure for Zero-Copy Exchange  
typedef struct DLManagedTensorVersioned {  
    uint32\_t version\_major;  
    uint32\_t version\_minor;  
    DLTensor dl\_tensor;  
    void\* manager\_ctx;  
    void (\*deleter)(struct DLManagedTensorVersioned\* self);  
} DLManagedTensorVersioned;

### **Zero-Copy Interoperability Protocols**

std::tensor implements bidirectional, zero-copy conversion functions to export memory buffers to external C libraries or import buffers from runtimes such as PyTorch, TVM, and CPython1:

#### **1\. Exporting std::tensor to DLManagedTensorVersioned**

During export, std::tensor transfers ownership of its underlying storage handle to the manager\_ctx pointer of a heap-allocated DLManagedTensorVersioned structure2:

C++  
template \<typename T, typename Extents, typename Layout, typename Allocator\>  
DLManagedTensorVersioned\* to\_dlpack(basic\_tensor\<T, Extents, Layout, Allocator\>&& src) {  
    auto\* managed \= new DLManagedTensorVersioned();  
    managed-\>version\_major \= 1; // DLPACK\_MAJOR\_VERSION  
    managed-\>version\_minor \= 0;  
      
    // Allocate context block holding src's underlying storage reference  
    auto\* ctx \= new tensor\_storage\_handle(src.extract\_storage());  
    managed-\>manager\_ctx \= ctx;

    managed-\>dl\_tensor.data \= ctx-\>data\_ptr();  
    managed-\>dl\_tensor.byte\_offset \= src.byte\_offset();  
    managed-\>dl\_tensor.device \= src.device\_context().to\_dl\_device();  
    managed-\>dl\_tensor.ndim \= static\_cast\<int32\_t\>(src.rank());  
    managed-\>dl\_tensor.dtype \= cxx\_dtype\_to\_dlpack\<T\>();  
    managed-\>dl\_tensor.shape \= ctx-\>shape\_data();   // Stored in context buffer  
    managed-\>dl\_tensor.strides \= ctx-\>stride\_data(); // Stored in context buffer

    managed-\>deleter \= \[\](DLManagedTensorVersioned\* self) noexcept {  
        if (\!self) return;  
        auto\* ctx \= static\_cast\<tensor\_storage\_handle\*\>(self-\>manager\_ctx);  
        delete ctx; // Releases reference-counted memory handle  
        delete self;  
    };

    return managed;  
}

#### **2\. Importing DLManagedTensorVersioned into std::tensor**

During import, std::tensor wraps the incoming DLManagedTensorVersioned structure. Its custom storage deleter invokes the source framework's C deleter function once all reference counts reach zero2:

C++  
template \<typename T\>  
dynamic\_tensor\<T\> from\_dlpack(DLManagedTensorVersioned\* dlpack\_tensor) {  
    if (\!dlpack\_tensor) throw std::invalid\_argument("Null DLPack tensor pointer.");  
      
    // Verify ABI version compatibility  
    if (dlpack\_tensor-\>version\_major \!= 1) {  
        if (dlpack\_tensor-\>deleter) dlpack\_tensor-\>deleter(dlpack\_tensor);  
        throw std::runtime\_error("Incompatible DLPack ABI version.");  
    }

    // Wrap external buffer with custom deleter calling DLPack deleter callback  
    auto storage \= make\_dlpack\_shared\_storage(dlpack\_tensor);  
    return dynamic\_tensor\<T\>(storage, dlpack\_tensor-\>dl\_tensor);  
}

### **Property Mapping between std::tensor and DLPack ABI Fields**

| Proposed std::tensor Property | Corresponding DLPack C ABI Field | Mapping Mechanics and Conversion Rules |
| :---- | :---- | :---- |
| tensor::data() | DLTensor::data | Pointer to raw memory allocation without applying byte offset24. |
| tensor::byte\_offset() | DLTensor::byte\_offset | Offset in bytes from allocation start to the first valid element2. |
| tensor::rank() | DLTensor::ndim | Total rank count represented as a 32-bit signed integer2. |
| tensor::shape() | DLTensor::shape | Pointer to array of int64\_t values specifying dimensions along each axis2. |
| tensor::strides() | DLTensor::strides | Pointer to element-based stride values per axis (never null in DLPack ![][image11])2. |
| sizeof(T) & Type Category | DLDataType (code, bits, lanes) | Bitwise mapping to standard scalar codes (uint, int, float, bfloat)20. |
| tensor::device() | DLDevice (device\_type, device\_id) | Enum conversion matching target hardware execution contexts2. |
| Memory Lifetime Management | deleter / manager\_ctx | Custom deleter invocation ensuring safe cross-framework deallocation2. |

## **Mathematical and Algorithmic Integration**

To support high-performance numeric processing, std::tensor integrates with existing C++ numeric algorithms, vectorization utilities, and dense linear algebra interfaces6.

### **Interoperability with std::mdspan and std::linalg**

std::tensor provides explicit view extraction functions to generate non-owning std::mdspan instances5:

C++  
std::tensor\<float, std::extents\<size\_t, 128, 64\>\> A\_tensor({128, 64});

// Generate a non-owning std::mdspan view over tensor memory  
std::mdspan A\_view \= A\_tensor.to\_mdspan();

// Pass views directly into standard dense linear algebra routines  
std::tensor\<float, std::extents\<size\_t, 64, 32\>\> B\_tensor({64, 32});  
std::tensor\<float, std::extents\<size\_t, 128, 32\>\> C\_tensor({128, 32});

std::linalg::matrix\_product(  
    std::execution::par\_unseq,  
    A\_tensor.to\_mdspan(),  
    B\_tensor.to\_mdspan(),  
    C\_tensor.to\_mdspan()  
);

This design allows std::tensor to serve as an owning storage backend for std::linalg routines6.

### **Multi-Dimensional Indexing and Slicing**

std::tensor supports variadic multi-parameter operator\[\] syntax (standardized in C++23) for element access5:

C++  
std::dynamic\_tensor\<double\> t({3, 4, 5});

// Direct variadic element access  
t\[1, 2, 3\] \= 3.14159;

// Integrated submdspan subview creation  
auto slice\_view \= std::submdspan(  
    t.to\_mdspan(),   
    1,   
    std::full\_extent,   
    std::strided\_slice{.offset \= 0, .extent \= 5, .stride \= 2}  
);

### **Broadcasting Rules and Array Arithmetic**

std::tensor implements standard multidimensional broadcasting rules for binary element-wise operations. When evaluating ![][image12] for tensors of shape ![][image13] and ![][image14], the indexing engine computes an output domain shape of ![][image15] using broadcast stride rules20:  
![][image16]  
Setting the stride along axis ![][image17] to zero broadcasts scalar values across that dimension without memory duplication. Element dereferencing maps directly to std::simd vector registers, maximizing vector execution throughput18.

## **Technical Considerations and ISO Committee Trade-offs**

Designing std::tensor requires addressing specific technical challenges identified during standard committee reviews of std::mdspan and std::mdarray5.

### **Resolving LEWG Container Adapter Feedback (P1684 / P3308)**

Reviews by the Library Evolution Working Group (LEWG) raised questions regarding constructor overloading and moved-from object states in owning multidimensional containers8. std::tensor addresses these points through specific design mechanisms:

1. **Moved-From State Guarantees**: Moving a std::tensor clears its shape descriptor and sets its internal storage handle to nullptr. Post-move operations, except destruction and assignment (operator=), trigger precondition violations enforced under standard hardening guidelines (P3471)8.  
2. **Disambiguation via in\_place\_t Constructors**: To prevent constructor ambiguity between flat storage initializers and multidimensional shape parameters, std::tensor provides std::in\_place\_t constructors8:  
   C++  
   // Constructs flat element storage in place using forwarded arguments  
   std::tensor\<float, std::extents\<size\_t, 4, 4\>\> t(  
       std::in\_place,   
       {1.0f, 2.0f, 3.0f, /\*...\*/ 16.0f}  
   );

3. **Initializer List Deduction Guides**: Deduction rules support direct construction from nested initializer lists for static rank tensors8:  
   C++  
   // Class Template Argument Deduction infers tensor\<int, extents\<size\_t, 2, 3\>\>  
   std::tensor A \= {{{1, 2, 3}}, {{4, 5, 6}}};

### **Static Template Specialization vs. Runtime Type Erasure**

Deep learning engines require type-erased runtime tensors (at::Tensor), whereas high-performance C++ code relies on compile-time static typing (std::tensor\<T, Extents\>).  
To support both paradigms, std::tensor uses static layout engines by default. For runtime execution pipelines, a type-erased container class, std::any\_tensor, wraps an underlying std::basic\_tensor instance and provides runtime dynamic dispatch:

C++  
namespace std {

class any\_tensor {  
    std::shared\_ptr\<void\> storage\_ptr\_;  
    DLDataType dtype\_;  
    device\_context device\_;

public:  
    template \<typename T, typename Extents, typename Layout\>  
    any\_tensor(basic\_tensor\<T, Extents, Layout\> t);

    DLDataType dtype() const noexcept { return dtype\_; }  
      
    template \<typename T\>  
    basic\_tensor\<T, dynamic\_rank\_t\>& cast() {  
        if (cxx\_dtype\_to\_dlpack\<T\>() \!= dtype\_) throw std::bad\_any\_cast();  
        return \*static\_cast\<basic\_tensor\<T, dynamic\_rank\_t\>\*\>(storage\_ptr\_.get());  
    }  
};

} // namespace std

### **Memory Alignment and SIMD Vectorization**

Vectorized code generation requires aligned memory access guarantees10. std::tensor ensures alignment by integrating overaligned allocations with padded memory layouts (layout\_left\_padded and layout\_right\_padded via P2642)11.  
This guarantees that the leading dimension stride (![][image18]) aligns with hardware vector register boundaries (such as 64-byte alignment for AVX-512 or ARM SVE), avoiding performance penalties from unaligned memory loads10.

## **ISO Standardization Roadmap**

To progress std::tensor through the ISO C++ standardization process for target inclusion in C++29, proposal work is structured into three execution phases:

| Standardization Phase | Technical Scope & Deliverables | WG21 Working Group Focus |
| :---- | :---- | :---- |
| **Phase 1: Core Tensor Mechanics** | \- Standardize std::basic\_tensor, std::tensor, and std::dynamic\_tensor. \- Implement variadic operator\[\] indexing and CTAD deduction guides5. \- Integrate std::mdspan view extraction and submdspan slicing support5. | LEWG (Library Evolution) & LWG (Library Working Group) |
| **Phase 2: Heterogeneous Support** | \- Integrate std::pmr::memory\_resource support8. \- Formally define device\_context and hardware memory domain semantics. \- Extend parallel algorithms (std::execution::par\_unseq) to support asynchronous tensor operations10. | SG14 (Low Latency) & SG19 (Machine Learning) |
| **Phase 3: C ABI DLPack Bridge** | \- Standardize native \<dlpack.h\> header bindings. \- Implement to\_dlpack and from\_dlpack zero-copy conversion routines2. \- Verify cross-language interop across PyTorch, TVM, TensorFlow, and OpenCV1. | LEWG & International Standardization Committee |

## **Architectural Synthesis**

The proposed std::tensor library resolves data structure fragmentation across C++ numeric computing libraries. By combining the zero-overhead abstraction model of std::mdspan5 with dynamic runtime rank flexibility, heterogeneous memory awareness, and native C ABI exchange via DLPack2, std::tensor provides a foundational, owning multidimensional container for the ISO C++ Standard Library. This standardized abstraction enables seamless data exchange across machine learning libraries, scientific computing packages, and cross-language runtime environments while preserving C++ compile-time performance guarantees.

#### **Works cited**

> 1. Creating an Application \- NVIDIA Docs, [https://docs.nvidia.com/holoscan/archive/0.5.1/holoscan\_create\_app.html](https://docs.nvidia.com/holoscan/archive/0.5.1/holoscan_create_app.html)  
> 2. C API (dlpack.h) \- DMLC, [https://dmlc.github.io/dlpack/latest/c\_api.html](https://dmlc.github.io/dlpack/latest/c_api.html)  
> 3. \[RFC\] Adopt DLPack as cross-language C ABI stable data structure for array exchange \#1, [https://github.com/data-apis/consortium-feedback/issues/1](https://github.com/data-apis/consortium-feedback/issues/1)  
> 4. Tensor and DLPack — tvm-ffi, [https://tvm.apache.org/ffi/concepts/tensor.html](https://tvm.apache.org/ffi/concepts/tensor.html)  
> 5. MDSPAN \- Open-std.org, [https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p0009r18.html](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p0009r18.html)  
> 6. What new feature would you like to see in C++26? : r/cpp \- Reddit, [https://www.reddit.com/r/cpp/comments/1bqv7w8/what\_new\_feature\_would\_you\_like\_to\_see\_in\_c26/](https://www.reddit.com/r/cpp/comments/1bqv7w8/what_new_feature_would_you_like_to_see_in_c26/)  
> 7. A free function linear algebra interface based on the BLAS \- Open-std.org, [https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p1673r13.html](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p1673r13.html)  
> 8. mdarray design questions and answers \- Open-std.org, [https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3308r0.html](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3308r0.html)  
> 9. WG21, aka C++ Standard Committee, May 2024 Mailing (pre-St. Louis) : r/cpp \- Reddit, [https://www.reddit.com/r/cpp/comments/1cy8k8o/wg21\_aka\_c\_standard\_committee\_may\_2024\_mailing/](https://www.reddit.com/r/cpp/comments/1cy8k8o/wg21_aka_c_standard_committee_may_2024_mailing/)  
> 10. MDSPAN \- A Deep Dive Spanning C++, Kokkos & SYCL, [https://nwcpp.org/talks/2023/MDSPAN.pdf](https://nwcpp.org/talks/2023/MDSPAN.pdf)  
> 11. cpp-proposals-pub/layout\_padded/layout\_padded.bs at master · ORNL/cpp-proposals-pub \- GitHub, [https://github.com/ORNL/cpp-proposals-pub/blob/master/layout\_padded/layout\_padded.bs](https://github.com/ORNL/cpp-proposals-pub/blob/master/layout_padded/layout_padded.bs)  
> 12. Initialise 1D vector using 2D vector \- Programming \- Arduino Forum, [https://forum.arduino.cc/t/initialise-1d-vector-using-2d-vector/1004096](https://forum.arduino.cc/t/initialise-1d-vector-using-2d-vector/1004096)  
> 13. Future-proof submdspan\_mapping \- Open-std.org, [https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3663r1.html](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3663r1.html)  
> 14. Fix submdspan for C++26 \- Open-std.org, [https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3355r0.html](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3355r0.html)  
> 15. \[DOC\] mdspan/mdarray quick-start tutorial · Issue \#654 · NVIDIA/raft, [https://github.com/NVIDIA/raft/issues/654](https://github.com/NVIDIA/raft/issues/654)  
> 16. span for projections? : r/cpp \- Reddit, [https://www.reddit.com/r/cpp/comments/18xgviv/span\_for\_projections/](https://www.reddit.com/r/cpp/comments/18xgviv/span_for_projections/)  
> 17. Future-proof submdspan\_mapping \- Open-Std.org, [https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3663r3.html](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3663r3.html)  
> 18. C++26 \- ISciNumPy.dev, [https://iscinumpy.dev/post/cpp-26/](https://iscinumpy.dev/post/cpp-26/)  
> 19. mdarray: An Owning Multidimensional Array Analog of mdspan \- Open-std.org, [https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p1684r5.html](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p1684r5.html)  
> 20. Tensors — augpy documentation, [https://augpy.readthedocs.io/en/latest/cpp/tensor.html](https://augpy.readthedocs.io/en/latest/cpp/tensor.html)  
> 21. ISOCPP std-proposals List: Re: \[std-proposals\] Interest in Linear, [https://lists.isocpp.org/std-proposals/2023/04/6371.php](https://lists.isocpp.org/std-proposals/2023/04/6371.php)  
> 22. pytorch | Terra Incognita, [https://blog.christianperone.com/tag/pytorch/](https://blog.christianperone.com/tag/pytorch/)  
> 23. Report from the Croydon 2026 ISO C++ Committee meeting \- mp-units, [https://mpusz.github.io/mp-units/HEAD/blog/2026/03/28/report-from-the-croydon-2026-iso-c-committee-meeting/](https://mpusz.github.io/mp-units/HEAD/blog/2026/03/28/report-from-the-croydon-2026-iso-c-committee-meeting/)  
> 24. C++ API Reference (Extras) \- nanobind documentation, [https://nanobind.readthedocs.io/en/latest/api\_extra.html](https://nanobind.readthedocs.io/en/latest/api_extra.html)  
> 25. Cologne 2019 LEWG Summary \- Open-std.org, [https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/n4823.pdf](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/n4823.pdf)  
> 26. P1684 R5 mdarray: An Owning Multidimensional Array Analog of mdspan · Issue \#461 · cplusplus/papers \- GitHub, [https://github.com/cplusplus/papers/issues/461](https://github.com/cplusplus/papers/issues/461)  
> 27. 2023-11 Kona ISO C++ Committee Trip Report — Second C++26 meeting\! : r/cpp \- Reddit, [https://www.reddit.com/r/cpp/comments/17vnfqq/202311\_kona\_iso\_c\_committee\_trip\_report\_second/](https://www.reddit.com/r/cpp/comments/17vnfqq/202311_kona_iso_c_committee_trip_report_second/)

[image1]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAIAAAAAYCAYAAAAyC/XlAAAC/UlEQVR4Xu2YS8hPQRjG39xKSG7JtYQSUe6U+Ni4lCjFAilFklyyYMGSWLCSS9nZkIRiZWfhktxXSmxEsmGBhQXvY2YYz3dmvnP5f//zP5pfPZ0zz3vOXM7MmXNmRBKJRIK5ykaio7nIRhU+spFoBD/ZKMNt1So2LeelRYV4bBaT52sONJwf0vpn1RPzVB/YLEqs0jNUW9msyDDVWdUADjScQ6pxbLYB9N9QNvNyVHWBzUSj2CcVZgGMnrlsWlaqFrFZkdmq1ap+HGg4e8TMbHXQX+KzeJSsG4eoHqoGi4njO9MKzqkmqb5JdrlN5Ys91tmm0mVn3fhetdOeIz7ei1Xhkj0iz6xym8hBVV/VNKm3TaXLjt24W7Ljx1Q3pdzAmC4mz9McULax0SDeqZ6RN19MW3eRX5STbGSAckawmYesDnZkdZR//Qkxn4ki4EE9Jm+N6oDE69LpoO6j2FTeqvqwmZMbqr2qexzIoPSzi92I2Bzyvnvn+Enc4aXzgDz3s2mJ1aWTQceH6h7y84JZpJYBsFH+xm7Z4xLVC3sOJqteeWnwRsx1WYyVf8vDbOATqkuXmP+SEIslfC+oGu+SePl4Jm4Zxvm49CYxy7Wi9PoA+Kpaz6ayQUymU1XHrbdF9fTPFd07FKsHpKEpnu8YLSaG6/APwVNjqBF3xcS2c8Di4qFlWKviofKfqM6I2VMZ4/kLVddUl8XkHWpfjDwDYLiUy/s3WL8+YtOCjvJ3mJarnnvpiWKWdAwqs4xNC3b/Qj+PPTXiCBttJlb+LOk+oLHFjjahE30wUNyLwvrkXQdw733ymFOqw2wWoacH70DnffbSS1VXvLTjDhs5idVjnWogm22kTPmuPfiEYv9jphfLCwbAAzaJ2HPLxQoxU1Ue/DceHT3ISzuv6MrAEWsIPlV1UqZ8157r9uj2QYqAl8z/72Iwg7tPdCVQuQlsBlgr5oeGpzywgI0WUPe2cdnyR3rnvJpqBZiRX7JZhdB3O9GZ9MbLlkgkEolE4j/mF3JuocOHsyJ/AAAAAElFTkSuQmCC>

[image2]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABIAAAAZCAYAAAA8CX6UAAAA1UlEQVR4XmNgGDHAAYhrkTAfiiwDQyWSHAiXokqjAjkg/g/Fd9DkmIBYFSpXgCaHAUSB2BmIlzNANGADD9AFsIEgIOYAYicGiEHKqNJgsAhdABs4i8QGGfQRiQ8CAUCshiaGFfxEYk9ggBjGiCR2BImNF7Sh8UEGrYOyXaF8ggAUPtxoYo8YEJoPAfEChBRucAZdAAiEGSAGJUJpFVRp7ACXs0Hix4H4ProELnAaXQAKzjNADEtCl8AGooG4C10QCkDpCpdrUQBIETLGBnCJj4JRMPgAALsDLHYtN1I4AAAAAElFTkSuQmCC>

[image3]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAkAAAAbCAYAAACuj6WAAAAAcUlEQVR4XmNgGAWkghwgZoSynYA4BUkODH4DsSkQ/wfiE0CsAMQbgLgPSQ3DbCA2ZoAoug8VA7GnwBRYAjELEE+DSvBDxSthCpABSEEPuiA6ACnyRBdEByBFeIEzAxGKZgLxNnRBdMABxKLogqOABgAAJmMSgn2jKIYAAAAASUVORK5CYII=>

[image4]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAL4AAAAYCAYAAABEMUduAAAEpElEQVR4Xu2aW8hVRRTHV2J5q1DUzKIkAkFQQQVD8eH0JdoFLy9hmL6oICqKeBcSwXtlgWgXKtAMoqyQIhNUFEQCNQh9UER9EksLQouUktD1Z2b4luvMvpyzZ5/TOd/84A8z/9l7z56918yemXOIIpFIJBL5P/KVNiJdisPaKMq6GtQsrmvDMpyq79GnDndCAPS1fXqD9aA7ocV5harb59Oz7oSSwPO8oc0ioCcdZd21+tF6Tv+JsmZwgDVZm5anWBUy93bNpqVetWWLKBwV6nweO21e6g6Ze2kXRlBnm0/YtNQCW4bjymYZa782i9CXOl/mKFUGUGGzAj9PvThmjjYt77BGa7MgaQNBP9Y32mxx8GVFe5NG9VPaKBHcR7Bpjy/w+5P5hDmSXnSZYMrwoTYVi6n63sazetk0Om1IupE/8IeJ9BKRbgf2UXV7e4t00FE4g6+p+l7qxhf4M23e8S/rCZEvmwFk6s8arf+g6gdxk9Xdpl+UBQFYTqa+88JDZ1ht0+hwA0VZO+Dr6J+K9HyRLpvnydxL0tenJmTguzm+r7GNZD3lqx/HYNHTwVrL+pW16r4jwoK6UOeXZNYe75JZB7mO1o6gvT+RecYbWd9br1lcZm3RZj3IwH+PzBSn1sDHi6/UoCy+pez6USeOwZToBTKLWOTL3FHB9S+QacM01krrtSvPkWnfVNbLrKWsI6yD8qAGg4EZA09hfFMdjGbyhfZU+bL5mbLrwwivj/lF5f8S6c2s7SJfK+hQqG+G8n8XaXQKCZ5tmR2xbBDg+hljB0dOQfEFCAWm2Flg3Xdam/XgC3zMVSvuAOZtMj29UXxH1Q9c8w+lHzObNc6m5XEXWQ+LfF7WU3p9mOvLIHiTTMfDoNGqoL2YyiXxEauHNuvgc9b7ZAanLLCY/kGb9YAtOBf4Y1SZA3vTH2hTgC0vuf+fpSx2U3qQgayXggB33BZpnDdP5PNyjNLvaS6Z6YDkT1Yf5bUSaO9JbQqw6RGKFaxt2vSA94COUpiswHdz56QfksrAN43RoBxfIh8TqXPaM4F1VpThPLkrA/ByFypP456RD3whfWVJgY8f37AjlbQofoz813NklT9O6eVZ9YNHyVxjii6w4B1he9GBYHyA9QzrM+HnJW/gX6EA29TuZeYRGtUo8Pn8mzVd+a+TWXzjU4d7+sTmnY5bH3rSnjOLzJrB4colPs+B624iU37J5qWu2rJd7gRBUuDjk45z9ijf4crxlwEfW8mUv6QLLJhmoTxpsEqrHwGNdp0jc8wGm3faYX3IxcRe1hDWb6zBtmyQLctLnsBHPODaaR225cGPUyEWMdj7PSPyeHC3RN5Rxi+QCPy09cQhbTSYkPVjKowvrcZ1Ep8kCHx02DQQD/q8tiREIx+i+//ghGt+IfIOX2coCgL/EW1a0CEq2mwgoesv+q4Q+G9pU4E6XtNmO9JBYf6S7IIan2bM7+VP7mANa6TyQoDp2tPatKQtzBtB6PqLBj6mUR9rU4DdnEnabGewUEoKnlrAIm2sNi1JC7gyafb+fuj6h2ojIFgw79ZmV6CijUiXoqKNSCQSiUQikUgkEgH3APG9RP1QffhQAAAAAElFTkSuQmCC>

[image5]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAJ4AAAAZCAYAAADAMJcbAAADZklEQVR4Xu2aW8gNURTH/64hd0mIlFKUXJJ7OinJPUqe6COlKKQk8oZIeZAH8siLS/EiL95ILhGeXMoLohBPEhLr355x9llnZs6eyzfnzGf/6t+ZWWvPXrP3rNl7Zs8BPB6Px9MeLmqDp2PZLRqkjVk4JdqpjSVyFwU1xFMaf7QhLQdhKqH6KF8ZHBAd1caALSiggYrJqLe3J/EL5bZpIHLGm432XoikuCNgboyiOSsap40Vhzdw2W36IlqijWk4jfZNtUmJ5+lsjovuaGMVOAf/UlFleiPHwBFOs5kryMhQmJiLtAMmIX/A+It86fgOU+dM7agw20SPRZ+0oyRy5Y1r4tlJ2kpLg2Pi2I7oxFogmhZs03/G8uXhY/D7SPTMdlSYwTCJR1yuX3fAuOO10ZUwWcrkCKJj3rS26e9v7edhT/DLOvlC1RM4Zv1G9WUZMG5NG11pR+KdQHLMk2j2HxatEO0VvVc+F1ajuU4uC+wSfVP2KsE2cVG3KBbDvHDqvoqCZVZqoyvtSLz9iI/ZT/QOZlq0scszMTnVpOGy6LeycZpgQsedSxXguY/WxhxcEG2AW5+wzEJtdMU18W6l0PzgmDjWwsQcox3CdRjfKNFI0TKYjn1tlZkFk0hpYJ0c9cg92wG39nci+1A/d92mvLj0CcuM1UZXXBOvSIaJforWaQcaz4cjH9koehpsk0miz9Y+SWpHL9R9V0TXLB+JO46f8+ibqB0B91G/SaLgqE3/cO0IYJvoH6IdAa3i08dVAGK3idMf+46J+daypyGuT2xcykQSXqyki9ZdcLX9oTYKW2FWxTnC8esFYeI9+VcCmIDm8+UxH5TN5jnM0sN07UBzXSE1mOe/Q8oewrf3uGMJR+sk/3Ik+2tIjj8P5nPZS8vG2YRwSYqcDx0pSTovwtmoVZmOxfXEZ6Cxc7nk8sraD3GtT5N0HDs4vJjtIEv8zTA3dh6S+oRcgrlxKgmnB45mLtgdsUO0ydonXEm/qmyuJHVy0c9OackS/w3M40UekvqEtPJ3PPyi4MJ6mL9w3RA9UD7CZ8a0DIDpwFB6wXqqqEvZyiRr/DxJcRuNfRLFC9QfgypLF9ynkjWiuTCjm2aVNhRAX20omazxc/1rxIGv2lBV5miDp2OZog0ej8fj8Xg8nv+bvwmb1U4adGWYAAAAAElFTkSuQmCC>

[image6]: <data:image/png;base64,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>

[image7]: <data:image/png;base64,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>

[image8]: <data:image/png;base64,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>

[image9]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAALQAAAAZCAYAAACYTwQCAAAEcUlEQVR4Xu2aXcgUVRjHn0RFslIvDD9ACEyQRA2i9MJ4iUoI8Sov7bWXLkRU1DDtogsVP1P7IBQS0jSDDEtJCVIv/EQwTM1IiiAqEUGyEgqV0Of/njns2f87Z3bnY2ffHc4P/rD7f2b3Oc/MszNnzqxIIBAIBAJ5+IyNQOX4ho0i6FFdV11Sven4Pzivy+YGG8QTqn9U91TnIu9YLVwYnOd51WDVCnejCoA694ip87aUV+cQMcd6Igey8q/qb9WTqlGqNWKKsiqbAaojqhc4EPGAaruYbSxDxYz1P8crgrg8B8TkwsGuCrZOHH9LmXW+LgX12lIxX/Qg+dNUd6JY2RyW5Lz4AcbFv1CtZjMH4yU+D/D5nUh/qXOV1J88MmF/hQM5oGyWcguy/C9m+uMDY8IUgHlbzGWyKDZJfB7Qjv3SKvpLnSOkgHyHxHzJXYlv6p/ZKAGM5zU2Ix4RE8dZmtnCRk58ecA+NjqY/lRn7obGvBlfwnpD6udTZTFLzNkC82gfdoyYEuEqgl92Kzgq5eRpN26dX6teqQ+XCsbRzWZa7Fk6Tu6KRxzPSd/PJKkRu1Un2SQWSm1+b3WrboviiMvzVN0W1YDrhNpR53eqbWxmAWfEd6RvUc00YZGcUO1lM4HZYqZLGOezFCsSN0/Z+6RMsDiwQ9pX5+eSc126W/Uwm8pWqRWFaUlZnFHtYjNiDBsOGOfK6PVDYlY8poo5OFmmCovYiOADjUbH2vdax+skfHV+LMU19EzVJ2JOVo1A3lNspgEPCl5kUxmkuimmqMkUayVfqb5kM+ItNhwwzjnR6/fEjN2S5WnjFTYiuKFxNcElslMb2lfnAimuoXE8F0tzDX1QzBQ4M99L/cF3sXPrJNDsWDtsVo3ASsUFNiPw9MqHO068/ojepwFPrn5hMwLfxXf+y1XryOsEkurEMeA68/CMNL43AujHD9hMw2UxB+ma6lHHRzHwW/EoOQmscmCuimkD4xvPn6p5znts9y69d7Fz4R/Jt2CBH3FeaeE8Fl9DD5PkPHbd9SwHInA8EPed2TAFQxyrFHG8JCaOp4Bx+OrEVM3dZ9MjfSpmFQQNN9yJN0OzDY28T7OZhm/FDBbP0TF/xRdCuBTNd7YrE+THMpkLblbWq14VE/9LzBkfr7tqm/UCL6mhl0Ue7uwZ5MGVAI/Xr0pyHouvoYEvjyXpewHiM9h0QBzHLw7UgLjvyhZX52+q8+5GYhr5Zak9hd3vxJolTUNXDuzcPIXhsx/S+ziSGi0NaOgNbDoUlScrvitAGn4Sc1+VFTT0aTaJSarjbFYB/AHod9VYDjTJr6qLzvu4hh4t/jv8tKChN7IZUWSeLOCGfwqbGYjbh2lAQ2MG4APTJ0wFefpTKfLsRCw74o9X+PsrfiAuPeL/70JaMEZXTFF5suL+sPPwBxsp4H30fn24F/hL2KwaWJMcx2YK5qomsCnmbJDrxiMFZeXxMZKNjDzORoE8ptrJZlXpYiNQObrYCAQCgUAgEAgEAq3nPs3gRiQvGRZLAAAAAElFTkSuQmCC>

[image10]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACkAAAAYCAYAAABnRtT+AAAB4klEQVR4Xu2WTSgFURTHD0m+y0KxUE9JlMhKWVn4zgZlLSmlWLNUPhY2rKRsydbCUslCPhLZiSwUJSvERuKc5t43d/5zZ+48eaX41b+573/OnPm/edOdR/TP76UZjRgq0MiURVaDWpeyNlmTftnKO2sETQfPaCShnvXJKmblqvWeOore0p1BLlhFaCpqyDu3AwtMJeucVYaFKPrIGyYcqrWE1Rwrr9XwhELWNXiC/mJX6mgLKTxRwjuaR96gHFaLWi8FOvyedfAvWXXgmWyRd14nFhQSXt+cWKRpzVj3GDUTqW1bvDhcIYV91jSaiAxJqfWq4SO2O/kTIRdYu2iajJH7Qhrpazc+DykvjiQha8kx55QcDYomCvfNsm7BQ3TILiwYFFB4dgApxjYoHlk74MlzfAAeokN2YwGIzXBDjgammuw9M6x7NAEdshcLBiVkn59mnryGZSwo9KYubx5kmBzDyQ8p+3AUtkcphP7J58CfUL7sn1G4huuQ/VgwkOseoYlIiBfyw2oNmk0RRIU8o/A80bjZpJA3zgCaNspZK6wN1iirMViORC4sr8bvIn9kZIY8VllD3k4naGbAA+sVzWwg21M+mgmQX++OVYWFbPHBakPTQdTznFWm0IghhcY/f54vpKd4/j81tkgAAAAASUVORK5CYII=>

[image11]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADEAAAAVCAYAAADvoQY8AAAAf0lEQVR4Xu3OoQqDYBiFYYWNYViYd+fFbM1ikeWxKix7Ad6A0THWbF6B3XewdJL16PfAW77DD3+ShBA2raVBj66e1OnR0Z16KnRwdKGJbjo4OtKXKh2cHOhNDx0cZDTSS+4WSpqp1sHB79MjneVuo6EPnXQIYYdyuq4s/b8JQSyF9hR86y1ECwAAAABJRU5ErkJggg==>

[image12]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAF4AAAAZCAYAAAC4j5m6AAACNElEQVR4Xu2YPWgVURCFD/7hXyoRjEJEGy3SiATBzkBSWtgIlimCkE5EU6h1ipgiQkBEQbBJl0YstAyIhbWFmixEBBGijYIEMTOZu9mbk3279250t/B+cHi8M7Nv5s67+/MekEgkEp0yJvom+uP00vlHRNfypI7ZB+utbU6J7lboomjPZnYgOnBdzAL5Z0QZulloGYOiNXTbzx1Y/UPu/VHRqPNUQ86v5RHsgOMccDxFtwv10T5W3GsT3ojG2YzkI8rrX0Ux/CA0cZVNj+uI+LB/yEHRa9FNNO/nnegGm5Fo7d9sInLwP1CfeF50n80O0F6VS7Ce93uxUN7j7wz+HpvCF1hskgNMPyzxAQeI06LDbLbMCOw+pByD9R18LfVYxs4Gn9c+QP6i82fIL0WbqNvtoeSnWKhi4VNbPyP/ImLIsLPBP8T2tajm/KQ6mg6hbeZFfeRp39PkhZCh+eAHYHV/cgB2KdZY/vhdScjg9bl0is0W2YvtuyvXCy+vDH7OVulvlOcl/mV3TBVPYHVvcUA4gbB5bhCSeAV2je8KvVGdYxPW91s2A8jQfMd/gtW9wAFhGGHz3GAC9Ylf2eiBnmIxCkV3aBnaN1/3Q8jQfPBVg/0Ai81yoBfa/BKbsLv2Gpst8wvFr0OmaghVZGg2+N0or7kLxZlwkmK1PIMd+Bm2G7+LHm/JaJdXKBap8v//8P1cZ714HRniB8/1fOmz++0iNdGLDPGDTyQSiUQikUgkEv8x6+cptJzIqQTOAAAAAElFTkSuQmCC>

[image13]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAD0AAAAXCAYAAAC4VUe5AAACX0lEQVR4Xu2XTYhOURjHH0KRr2yURg4ipbEik8U02U3KVyJKIhZWaogsyGJ8LIQ0KwuRsrKyULLwkY+yIFkraxtJ1IzE/5lzrvfc/5wz97lnVpP3V//ee//Pc89zzr3n3nNekS5d/itOQJvYnCb8YsPCSugxm2CD+Ab/QC+hWfWwmZnQezaNHITuQQPQdug49Bnq7aTIDOgHtDHyGtFBMWugp9H5LUnnTcZp8de8C78l6CD12lhHaxmeh9KixjLoK5vgtvhG1ofzreF8/78MO05adIg4Bq1jM4HOpu/QFg6k+AKdZFN8I+ei8+qpLYo8K07KB31EbINWzkBv2EyhndnMZoK3Ut5xJ+XXthm0PmVTnaakvdBd8TOiFCfNdXIcgp5A16B94fhjLaOOqY4lSd9rzbvCASNObHVS6LdkNXna1hB5FaY6piSwWHzuLg4YcGKvY0HbyrWX82vkkhy0lryq2Arym3CSr9PEN+gweaOSby/n10glXZL03ay8HvKbcDKxLSt6HW+cUn2ryPk1NGkBeaugn9DVyNsjPjfe9VwO3oHIS+Ek3xnd6WlsOQcCuvbGzBOfv5R8ZYnk69TQqTLIJjgPvQrH1TZvrBMe56L4InfIr9ClZgT6IJ08Xfvj9f9FiOnSlOIUdD0c60ZKd176QFLslIk3KclZ6BmbgTnQDegBtJtiMY/YKED7kUNfp/vQBfHLVo7XUB+bOUxTIsN8qJ/NluwQf4Onwk1pOY5h8X8tS/jNRgH66kwV7cc2Npt4Di1k08BsNlpS+nc15hMbbRhgYxowV/wXvUuXwF8T6IIQbLbJPwAAAABJRU5ErkJggg==>

[image14]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACsAAAAXCAYAAACS5bYWAAABuElEQVR4Xu2WTSsFURjHH0lRXsrCaxELCysf4SYfwIZICUkhspVQysbLwoqylFKWYsHKRnwNK2SBYif+zz0z9878m3PO3JG7ur/61z2/eebcZ97OjEiFComsIv0sy8AjUsXSRR9yzpLYR45YephGTpAcMowsIM9IZ6RG+UGmyFnRYhe1YmpKbXZJzH7RjMUqDGvi7yFPB/LEkvgWM9khb/Awj/SwtHCNLLNkXpFFlhF2kCbJdmbnkF6WFgaRT5aMNjHAMkJ4ef672fBWc+Iq2EBag99Zmp1BbpBdZBK5RR5iFXFcveSxFVQjl5FxlmaHkG5yOo8eRBK2XgrYCj5onKXZJHQe23+qd665STs2S3HSpJwVS53oAzNBztesE29BQJYzq/tcJDhdCpPw9qIF+iT60LpjcveBbyEf8kXjRjH1DeRDvM3qUeoaZ2MEORUz0QuyHTjlLvDjwZjR74294Le+0nVleCtujtEmKZpdR65YlsgKiwhdYu7xTTHfBzYOxP1yKuA9IgejYpa5v6BrcOoe9FLNskzJO4sMaKNbLF3o/VfP0kMNiwzoi6eOZRpyLMpAO4sK5eIXsBBn+P0gC+4AAAAASUVORK5CYII=>

[image15]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAD0AAAAXCAYAAAC4VUe5AAACYElEQVR4Xu2XO2gWQRSFr09Q4gMbQeIbRRCtIkqKEOwkEB+IoiCiaGGlqCgBX4VJLETF0iIYBKtgISiIiA98gK21YG0jIgoqoucyM/6zJzO7M/NjEfw/OLBz5u7dubs7s7MiHTr8VxyHNrE5RfjJRgoroUdsgo1iEv6GXkIzq93ZPBGTK4cD0B2oHxqEjkEfoA2tEJkGfYV6PK+R0EDWQE+99i0Jx6WyTMz5uTm0SHee05FKhOG+ZOReAn1iE4yJSbLetrfa9r6/EXnoNUqKPgqtYzPAdOgL1MsdIT5Cp9gUk+S81z4jZsALPC8VnRpKSdGHJa1o5Sz0hs0QOojNbAZ4K/kDdugTUP510fqUk/I3Be2BxsW8ESU8gGbY45KiD0KPoWvQXnv8rhJRJSl/SpDOa427wh0NLIbOee2SonUtWU2e5jhJniMpf1IQWCgmdid31PCD2iVFh6jLE/MrxIJWQGvJcxdbTn6I7dKKD8lfJOv4DB0i77vExx3zK4SCRiR8N53XTX4qoZxNaDxvnOryxPwKGjSPvFXQN+iq5+0WE+vvekatt9/z6ggNVj9n6i0l3+FWfsdcMfG6XjCLZHL+IPqqbGMTXIBe2WO3zeM5OizmIrfJZ05A96RV9EUx20rlhfX00xTiNHTdHutGSnde+kBC7JDJNynIEPSMTcts6AY0Ae2iPp+HbBSg44ih0+kudEnMZyvGa2gLmzGSXokIXVAfm5nooqc3uB1uSmYdl8X8Wpbwi40CdOq0i45jgM0mnkPz2UxgFhuZtPu7qrxnI4d+NqYAc8Ss6B06WP4AAXuUE6h5QqMAAAAASUVORK5CYII=>

[image16]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAd8AAAB0CAYAAADErDjBAAAREElEQVR4Xu3dDbB9az3A8YeKhFJ5Sakupa4Rhkgk/39JSV6ivMWIRMSkKe/hXypFeb2RJrf77xLykpokhemK8laIKC5qLhNivNTEqDGs7zzr5/zO76z9du5Ze+9z/t/PzDNnr99ae5+919ln/dbzrOd5VmuSJEmSJEmSJEmSJEmSJEmSJEmSJEmSJEmSJEmSJEmSJEmSJEmSJEmSJEmSJEnalbcP5a9qUJIknbz3G8r/DuWpdYUkSZoHiffXa/AMedlQvqwGJUnalbe2nnzPqvccyl8P5XvrCs3uv2pAG7vDUJ49lF8byvuOse86WC3pNLpp64n3l+uKM+TaoXxPO9ufcR89rZ3tk7pteM1QnpSWH9T6Pv3sFJN0Cl1o/Z/50+qKM+LeQ3m3oTyk9drvrnBNndrKzeqKM+zTm8n3+njTUL6xBgcvqAFJp8+zWj9A3q6u2IJbDeXyGixe2fr7+6O6Yk0Xx5+f0OZPBO8cyl+2own2DUP5yaF8d+vvgSbEsy5OdObe52fVR7XF++7ba0DS6UNS4J/8tnXFFvz9UN5Rg8Xvt+Mn31sP5Q9b70j2O23xweyk8PqUe03EbzSUK8bHcybfhw3lM2rwhHEN8r1qsHj6+HPufT63bVxbfUQNDH6+9ZM2SWfUc1o/QN6mrtjQcQ6y/z2U/6nBCW9ux0u+f1yWj/MeN/E5Q/mIErtHm//3ZvyuH6rBE8ZwtFvWYPLS9Hibn30O23j/r62B1n/vw2tQ0tlxse0u+a7rb9pmyZfOKH8ylI9MsY9t877HRagFb/P3zp18ObngpGlR8n3jUL54KOfHss3PftJu3OZ//yTYqeTLPv68GhxcN5T3rkFJ6+Gf+pE1uCNXt+uffF/d5j1Icf1wk+S7C1yH45ruLw7lw8cYB88rW983NF+uasL8vqH851B+byjnUvwbhnLfodxnKF84xr619dcn0X3qUG4xlHe1/rt+u03/vge0PqzsX4by/Sn+ua1v+1Mpxu98buvDtAKd8nh9CsO2eA6vGe7ceseyjG1ra8C20J+hftZvGcrXtv5Z7j+UTx7KxwzlK1rfv49pvf8Df8P4rFP7El/V+mWTF7XDCfFRrW///HH5nq33tKencsYlAl6fyy/1d3xBOzrT3Fe31c39kpb43db/6eiFu2sccFcl35u03jzMAZmDUxyUEI9zCTn2H60fTEgQeHlal91xjHG96weH8m+tH5xq8qXmxXYkvVe13fZkBkmKWh/viVp2oLd1/YxT2Cb3OGd/R8cahoMxxpNtojMXSYRlenHH9+jcGJuq+XJSQGtAiOeHuPb/50N54Bi73xjL4u9Wa77xtyQZ8X3JMcrHjbFF7tYOasvrlFX4nbcfH9fPetvWv1/E3qMd1HCfN5T3T9t95xifwveN67Jg/7Nd/G1uMJRrxtjrxhhYzicrEZuq+eIJrfdZ4Gd+HUnHxD8czUr74Kdbfz/Lku9vtsPjCmtCiQPylMe1vo4z9h8YylvSOmpx+XkctFl+9xQDiSgn3x9tfbubpxjLT0nLu0ANi/exafKNE6CMBFhj1IqpZSHvx3Cu9efU5MuJAXGSQkbssvFxTC9Kos+IkZzCouS7L/isjOtmjvIsf9Yce2zrNX9qw9Wi5HuXdjROksw1VV6zbsPyKydii5KvpBPGPxydc/bB1a2/n2XJN4bIUHu6fVmHZcn3m9vidfTKzeuYDWlq25p82aZuR80tatW78sTW39ddU2yd5BufZ6pUEaemWNG8yTpaDDKaK+vrRokpN5k5adHvi5osogaeT3z2yTqfNUTryZ+VePiONr1P4mRpqoTPKstgmV73Gd9ZErekGbw+PX5w67Mt7Ys4kHxwXVHEUJ0oV6Z1y5IvNYtF6+IaYuAxTcwVB6ip5Ht+okxhKMc2xkbGSUqu+dbPOKUeuJeJqUBfXFe0g5rvD5d4vK9laJmY2oZYrg3H3zo3z07h+unUddK5xWf9rbpigWX7flHNl6FrU/GsnliCZa7n11i9pFJx8sM1fnrOS9pA/iekyfRD0/KurZt8aQq+0A4OVvkzHTf51p7APObacLVOzXcZpuirzX1z4KSK95VrvuvM8rTu56FJlVrSC9v09udaj/Mdw8+MP79ojC9Dh6GpbYjlTlTxt/7AcXnqOVgnQWW/1Ppz1i2Lmr3js+YT3kXuMJSrWt/+o8s61OT79ePPZ5T4lM9sR7dh+Q8mYnEt/uvyioS/67+2fg1e0pqiWTfQgWifxDVfZpta5NqyzEGC59xwXGZcZ3xGrofdaXyMZcm3JiZmh5raluSbx+yyzdR2tVkx0GGldnSZQyTfXPOll/LUe814b2xTO+DV5tD8OuyTP03LiFm8fmJcJqEF4vwtMjoe8TdAXPOtiOXkG4n/Q8blqecEWix2gV7j9X3lzxroCY3Ht6Pbo353o6c5iLO/s3ydedE135p8Odn8i/Hxo/OKYl/6iEinAh1cPr4d/SfcJ/Tw5P0tS75/O5RvSsuRLALJLZbpWZqTCM29iz5/7U3LcBmWc5Mm+5BYHChBLYWD1j+kGE1zF9Jyto0DF82db2v9vTIrFwd6hpyw74ixn7/k/7c+Kk7SvnIoH9B6k20ki5eM6/K++pVxmebn3Lwb27EP35ziDJ8hzvAb+huwD6OD0INab1lgPSdSXLtm+M2TxxgnPtQowRAcamH02L97m557GDQ712vP20QLwTvb0c9KYqZJms9FEz7i+iwnC+zLrxnj9IQm/ontoKNbiGv5dPLjejn7j+3BiIDrxvW0PvC95nXjb0PNnX0HhhQRoya/7A5Q+W8vnVo033GQW4WaB196Dk7HEdcveY0Pyyv2CAcH3t8H1RUJCeTbWq9t/V3rNdRai6DpjNfJPW3jYBMlJwkOhnldeJ8Uo8bNczgo1u3w9BS/kOIk7Ctarwlzbbo+b5+xn6d63m6C3sk0qU6hJs4+reNxN0VNsk72cE3r191J0OzzVZcy5sZJ4El81kiUU/iOcV3/+uD7mltLAv9rDL3757ZeM7q0VXxxNx3EHwdszkBXYbvjJF+apG40PqaWxtn1Pvq51j/jqg40pwmfh/GqoEZcm2918hgD/uNp+TSd8OwjJuiIlhIuZzCZirRX6Njw+TW4Ak1wNJfVa2xTjpN8H956s+D5sdBER3PsPlrWWeq04vPcY3xMzccD1/xonYg+ADhr36lty/tvG5dNpI3RfLlp8t3EcZJvbl4FvU/p2LSPohXgLMnDleLA9dAU08nL3yFOdmj2Z0ywjifvTx5zqWydyoJ0omjO4gtIE2keMvKzY5zERsJ75Bin5sky4x3Pt95Z6OK47lPGdcxNSyeZih6HTCbBuDpMJV86HNFxpc4bSyyS2YUx9iMpFtPR7RPeV+1Mctpx0P/V1sdU0vOX4VSa152H8szW5zC+vPU+ALU3sNZHb2mam+nXQMewOjmHNDsSLwdTBq2TvOKMkI4IkdSixLU9pmuLGDPe0CM0HsfYPQpJNqNnLp2K6BHJMI1Xt6PJN3rf/lPrNW4ex/i/04Yeqbz/GLN5VlBD4PsRNYVogta88vzNUx2ItJlPGn8yAcqt8wppGyLZLlte1OwcSRbMixvX/ujZW5MvPUzra6MmX5a5zhyYfGLqeSclPsM6ZdNrmzznYg1KkkSCoDPHw9rhDh2B9cuSL03VVUyl94IUY3lqXCLxSL6Lpgokdr4G9xhJ+u1tO1MuSpJOoWjmjfKqw6tXJl8mCqhI4lPJlzmAq5x8nzQun58oUxPN36b1u/rwc1/Q9M4wHMbTSpI0KV+vi1l68tzILDNLDOpt1lg31amK8bdTyZdb3lU5+a4zR27FNeTrg7HI6xbmSl4HnZD4HEzoIEnSETXZXdcO5pUF6790fFznTWbd40sMkXzzNV+mdqMTVZWTeyw/Ni2D2X7qjE+hvv99wnuzJ7Ak6QgSBMMXEDXfjDlcmUHqsnaQTOk8FTdcJ1nTXBzows9zWMcctblmTOyp42OuC79jjDG9W9QS4z1MzRsbGF70j61fI843BNg30cmMuWmlOSybr1jSHmPCBOYW5g4uNOHWWuetWk8gFCakR0wSn0vgpuuL1p0blxkfSu/ouEsM5d/Tdkyld+0Yf1frNd/w0NbH+PJeWJ+n3Ns30QLAWELppHFSl/+/JGkW9Ybkr2i7n1x+lXoCIp0Ubgzgd0vS7GiyzsN3TsOBJ26vpnlxZx1mVrtZXXFGxZhzv1uSZnexHdxhKZp0mc5yn8UUnevc1YibQ3CtmyZ5elfrMPYNt1usCfYNrV/uoH8B+/rZh1efSXEpY9fJ91Gt73tJZxxjaF/T+vzSHHh+4fDqvRPJd9n9fPHCdvjaMBO97/rAum/YH3S0y/ePjWv/8Zj5xumnMBcSfb6n8hy4h+0tazB5aXq8q+8Iv5fOXvw0+UraO89r/QC1KiFMHUSJMfOYFjvfpvfdXPhdcydfbpe5KPm+sfUm5/Nj2eZnn8Lvv7IGJWnXuCPUquTLkKqpgyjjqrmzkxa7d5ved3OIG31MTZN6UuJmI1MzunFno1zrB9vGpZhdMPlK2ktMsrEq+cZtESuub07FL0XshyhxR5+LJb5sX31560PpmNCFu2qx7Y3Hdfn5MeY9x+jkx3C8+rvy74tpWrl0UO/GFd8BCkmV2zFyq0yabV83boP62pQfK+sYG3+TEqPkOx5tE7/b5Ctp76yTfKNpuqKJcSp+KeIaeOyPfDu9dWu+bMOkLIFEHD3nb9r6bTXZJjpz0fmN5Ye0g1snnhtjU83OTEDDPXZDPD/E+Hf6LDxwjN1vjGUvH2OLmp2P627t6Hzpy8q6TL6S9tLVbXXyjabp6vVtOn6porZ4nOQbJ0AZCbDG6GX+ovFxnccci5JvdI6j9psRu2x8THMxyyT6jFjUwDFX8p0L7/U5NShJuxbJN8+XXV1oRxMBoqlT3RNb3x93TbF1ki/rF5Uq4tQUq3u2vq5e8+VOVvV1o9BzGYtmoyIWzciIGvjUNd99xHu9qgYladfWSb70Xp06MNPhaip+qYpxvLnmu+ge0FkkwnW8tfVtX1xXtIOaL8OZsnhfy9TZ2QKxXBuOmu+qceH3aX1SkV3jvT63BiVp19ZJvlh0YH5aDV7Cpmq+9x1jy1zTpre5e1lmco7LWh+TzfZ3ObS2T+hC/BnjMmO4cYsxPlVbjd7J3N956j0Qyz2YGcNLLC5TTD0nLFtXkag3KauSf+A98B2XpL1CrWCd5MuNJejck/G86Oyj1p7c+j7JNd/7j7FV2IZe5Rl33QqPbr13eXhcO/q6UXt9xbicb7HJ3bXoxJXl20nGNd+qJu0njDFqtnhTWpdxnZgblOwa79XbZkraO1e19ZIv2O75Q/mN8TF3fVLH/siF4ULcajLHmCVsmZhnm5Kn78yvMRXLcWrfTHNJrNacuRtXbJ/vxpWHGkVh6FNezve+5nozMX5PPvmiQ9c1rY/9Zp7zB6d121Y/D+W1h7aQpB26svUD0zrJV1qEuczflpb5TkmSFuA6IgfK29UV0gaoVed7V5t8JWmJp7R+oMwTPEibohn7hmnZ5CtJS0RP2JfUFdIGcrJlaNoVrY8JliQtEB1SpOPipgrPbH3u6ctbn8rSDnmStATXe0m+F0tckiTNKKZBfERdIUmS5kUCflkNSpIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIkSZIk6ZL2f+t3vNIK+UutAAAAAElFTkSuQmCC>

[image17]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAoAAAAaCAYAAACO5M0mAAAAk0lEQVR4XmNgGJTAFIiXAvFudAl0MA+I/0MxQdDGQKTCv0C8GF0QGwCZpocuCAI7gPgsEH8G4gIGHNZmAvFFKFuMAYdHtLAI7gLij2hiYEX7sYhVoYmBBa2wiHGgiYEFeZH4zFAxDIAuWIck9gJZ4hwQJwGxOBD/AuJeBohCByBWQiiDADYg1kfig4KIB4k/CugJAK0XIfwiXGGPAAAAAElFTkSuQmCC>

[image18]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACwAAAAZCAYAAABKM8wfAAABkUlEQVR4Xu2Vuy+EQRTFD0EiIRrRSdQK70cUEkRB1BS0KlGLRitKnUqtoJKIaNCo/AkS0aATlQQR5ubOZO+ezMx+q55fcrO755w7e+d7AoVCljFXe5EatyGCs7aWTK4qV65+WWyENDy5mnHVXm9FuYf2tBitx9WJ13uNnqMbmm9q4E5owwobGST/xaLnDNUH+MY/Bl6ANshuqyL5GxY9o1C/g40IL6gN3EZekks0uUNofpZFg/gbLBIP/jMMPGS8LBKWU1OVLeQ3KEdK/F02DP2uDv33V2h+vWbnkfA+ixnCH6TYhvpTbBhs/63/fWC0JHPQcBfpOSR/waLhA/kN7aD+9B9D8+dGS3KN9OITrgZJk8eY5KdJt4h/yqJB/Fg92lCKT6QHPmIBepol38qGZxLqD5AemHe1SdoatOeH9CgSvGPRE1tALoXUBsPNxgNZYr3DqB3lLH3Q0CLpI15fJT08r99Jl3XevCd3f4pn6BqMvFkbDhwCuQrIDcmerdilY+G8HNGUJ7Vs/EKhUCgUmuMPAl9/taxC8gUAAAAASUVORK5CYII=>