package com.example.labstatus.ui.dashboard;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

public class DashboardViewModel extends ViewModel {

    private MutableLiveData<String> mText;
    public String text;

    public String color;

    public DashboardViewModel() {
        mText = new MutableLiveData<>();
        mText.setValue("This is SL1 fragment");

    }

    public DashboardViewModel(String t,String c) {

        text=t;
        color=c;
    }

    public LiveData<String> getText() {
        return mText;
    }
}